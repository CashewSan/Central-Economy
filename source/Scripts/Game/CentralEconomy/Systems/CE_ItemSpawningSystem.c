class CE_ItemSpawningSystem : GameSystem
{
	// To preface, this is a mess, but I'll try my best to keep comments for those reading lol
	
	protected ref array<ref CE_ItemData>				m_aItems 					= new array<ref CE_ItemData>; 					// storage of initial item data from config
	protected ref array<ref CE_Items>					m_aItemsToSpawn 				= new array<ref CE_Items>; 						// item data containing items that need to spawn
	protected ref array<ref CE_ItemData>				m_aSpawnedItems 				= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 		m_aComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref array<ref CE_Spawns> 				m_aComponentsForSpawn 		= new array<ref CE_Spawns>;  						// item spawning components in the world WITH NO LOOT
	protected ref array<CE_Spawns> 					m_aMatchedSpawns 			= new array<CE_Spawns>;							// spawns matched to item array
	protected ref array<ref CE_ItemData>				m_aItemsNotRestocked 			= new array<ref CE_ItemData>;						// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 				m_WorldValidationComponent;													// component added to world's gamemode for verification

	protected ref CE_ItemDataConfig					m_Config;																	// config containing item data (typically in server profile folder)
	protected CE_ELootTier							m_currentSpawnTier;															// current spawn tier that items are spawning at
	
	protected ref RandomGenerator 					m_randomGen 					= new RandomGenerator();							// vanilla random generator
	
	protected bool 									m_bHasGeneratedSpawnData		= false;											// has spawn data been generated?
	protected bool 									m_bHasGeneratedItemData		= false;											// has item data been generated?
	protected bool 									m_bWorldProcessed			= false;											// has the world been processed?
	
	protected float 									m_fTimer;																	// timer for check interval
	
	protected int									CHECK_INTERVAL; 																// how often the item spawning system will run (in seconds)
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
		
		CHECK_INTERVAL = 15;
		
		DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;

		if (m_fTimer < CHECK_INTERVAL)
			return;

		m_fTimer = 0;
		
		if (m_bWorldProcessed == true)
			ProcessData();
		else
			GetGame().GetCallqueue().CallLater(DelayedInit, 1000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	static CE_ItemSpawningSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}

	//------------------------------------------------------------------------------------------------
	protected void DelayedInit()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();

		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.GetWorldProcessed())
			{	
				m_bWorldProcessed = true;
				
				m_Config = m_WorldValidationComponent.GetItemDataConfig();
				
				GenerateItemData();
				
				ProcessData();
			}
			else
			{
				Print("[CentralEconomy] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
				return;
			}
		}
		else
		{
			Print("[CentralEconomy] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateItemData()
	{
		if(!m_Config || m_Config && m_Config.m_ItemData.Count() == 0)
			return;
		
		m_aItems.Clear();
		
		foreach(CE_ItemData item: m_Config.m_ItemData)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!item.m_sName || !item.m_sPrefab || !item.m_iNominal || !item.m_iMinimum || !item.m_iLifetime || !item.m_iRestock || !item.m_ItemCategory || !item.m_ItemUsages || !item.m_ItemTiers)
			{
				Print("[CentralEconomy] " + item.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			
			m_aItems.Insert(item);
		}
		
		Print(m_aItems.Count());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessData()
	{
		SetHasGeneratedSpawnData(false);
		SetHasGeneratedItemData(false);
		
		GetNextSpawnTier();
		
		GenerateSpawns();
		GenerateItems();
		
		DataCheck();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DataCheck()
	{
		if (GetHasGeneratedSpawnData() == false || GetHasGeneratedItemData() == false)
		{
			GetGame().GetCallqueue().CallLater(DataCheck, 1000, false);
			return;
		}
			
		GetGame().GetCallqueue().CallLater(TryToSpawnLoot, 3000, false); // called later to help lower the blow to performance...?
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ELootTier GetCurrentSpawnTier()
	{
		return m_currentSpawnTier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentSpawnTier(CE_ELootTier tier)
	{
		m_currentSpawnTier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetNextSpawnTier()
	{	
		switch (GetCurrentSpawnTier())
		{
			case null:
				SetCurrentSpawnTier(CE_ELootTier.TIER1);
				break;
		
			case CE_ELootTier.TIER1:
				GetCurrentSpawnTier() = CE_ELootTier.TIER2;
				break;
			
			case CE_ELootTier.TIER2:
				GetCurrentSpawnTier() = CE_ELootTier.TIER3;
				break;
			
			case CE_ELootTier.TIER3:
				GetCurrentSpawnTier() = CE_ELootTier.TIER4;
				break;
			
			case CE_ELootTier.TIER4:
				GetCurrentSpawnTier() = CE_ELootTier.TIER1;
				CHECK_INTERVAL = 120;
				break;
		}
		
		//Print(" " + GetCurrentSpawnTier() + " " + CHECK_INTERVAL);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateSpawns()
	{
		if (m_aComponents.Count() == 0)
			return;
		
		m_aComponentsForSpawn.Clear();
		m_aComponentsForSpawn.Reserve(m_aComponents.Count());
		
		foreach(CE_ItemSpawningComponent comp : m_aComponents)
		{
			if (!comp.GetHasItemSpawned())
			{
				if (comp.GetSpawnerTier() & GetCurrentSpawnTier())
				{					
					int combinedCategoryFlags = comp.m_Categories;
					
					CE_ELootCategory spawnCategory;
					
					if (combinedCategoryFlags == CE_ELootCategory.CLOTHING)
					{
						spawnCategory = CE_ELootCategory.CLOTHING;
					}
					else if (combinedCategoryFlags == CE_ELootCategory.FOOD)
					{
						spawnCategory = CE_ELootCategory.FOOD;
					}
					else if (combinedCategoryFlags == CE_ELootCategory.TOOLS)
					{
						spawnCategory = CE_ELootCategory.TOOLS;
					}
					else if (combinedCategoryFlags == CE_ELootCategory.WEAPONS)
					{
						spawnCategory = CE_ELootCategory.WEAPONS;
					}
					else if (combinedCategoryFlags & CE_ELootCategory.CLOTHING | CE_ELootCategory.FOOD | CE_ELootCategory.TOOLS | CE_ELootCategory.WEAPONS)
					{
						array<CE_ELootCategory> allValues = {};
						
						SCR_Enum.GetEnumValues(CE_ELootCategory, allValues);
						
						spawnCategory = allValues.GetRandomElement();
					}
					
					CE_Spawns spawns = new CE_Spawns(comp, comp.GetSpawnerTier(), comp.GetSpawnerUsage(), spawnCategory);
				
					m_aComponentsForSpawn.Insert(spawns);
				}
			}
		}
		SetHasGeneratedSpawnData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedSpawnData(bool generated)
	{
		m_bHasGeneratedSpawnData = generated;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedSpawnData()
	{
		return m_bHasGeneratedSpawnData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Spawns> GetGeneratedSpawnData()
	{
		return m_aComponentsForSpawn;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateItems()
	{	
		// for every item in entire loot pool
		foreach(CE_ItemData itemData: m_aItems)
		{
			// that matches current tier being processed
			if (itemData.m_ItemTiers & GetCurrentSpawnTier())
			{
				int itemTargetCount = DetermineItemTargetCount(itemData, itemData.m_iMinimum, itemData.m_iNominal);
				
				CE_Items item = new CE_Items(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
				
				if (!m_aItemsToSpawn.Contains(item))
				{
					// clamping the min here to item.m_iMinimum meant it would never stop spawning an item past nominal even if itemTargetCount is 0 for items with a non-0 min
					for (int i = 0; i < Math.ClampInt(itemTargetCount, 0, itemData.m_iNominal); i++)
					{
						m_aItemsToSpawn.Insert(item);
					}
				}
			}
		}
		
		SetHasGeneratedItemData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedItemData(bool generated)
	{
		m_bHasGeneratedItemData = generated;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedItemData()
	{
		return m_bHasGeneratedItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Items> GetGeneratedItemData()
	{
		return m_aItemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal)
	{
		array<ref CE_ItemData> itemsNotRestocked = GetItemsNotRestocked();
		array<ref CE_ItemData> spawnedItems = GetSpawnedItems();

		int itemTargetCount = nominal;
		int itemCount = 0;
		
		// for every item spawned in the world
		foreach (CE_ItemData spawnedItem : spawnedItems)
		{
			string name = spawnedItem.m_sName;
			
			// increase item count if there is a name match (should probably check something more unique like prefab or even better a UUID!)
			if (name == item.m_sName)
			{
				itemCount = itemCount + 1;
			}
		}
		
		// set max count to be the difference between nominal and spawned counts (e.g. 7 nominal - 2 spawned = 5 max)
		int maxTargetCount = nominal - itemCount;
		
		// set min count to be the difference between minimum and spawned counts (e.g 2 minimum - 0 spawned = 2 min)
		int minTargetCount = Math.ClampInt(minimum - itemCount, 0, nominal);
		
		// @TODO: turn itemsNotRestocked into map for more efficient direct lookup by name (or something better like UUID) rather than looping through whole array
		foreach (CE_ItemData itemNotRestocked : itemsNotRestocked)
		{
			// increase min item count by number of this item awaiting restock
			if (itemNotRestocked.m_sName == item.m_sName)
				minTargetCount = Math.ClampInt(minTargetCount + 1, 0, nominal);
		}
		
		// set target count to a random number between min and max
		return Math.ClampInt(m_randomGen.RandIntInclusive(minTargetCount, maxTargetCount), 0, nominal);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_ItemData> GetItemsNotRestocked()
	{
		return m_aItemsNotRestocked;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineAvailableSpawnCount(CE_ItemData item)
	{
		array<CE_Spawns> availableSpawns = {};
		
		foreach (CE_Spawns spawns : GetGeneratedSpawnData())
 
		{
			if (item.m_ItemTiers & spawns.Tier &&
			item.m_ItemUsages & spawns.Usage &&
			item.m_ItemCategory & spawns.Category)
			{
				availableSpawns.Insert(spawns);
			}
		}
		return availableSpawns.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<CE_Spawns> DetermineItemPool()
	{
		array<CE_Spawns> matchingSpawns = new array<CE_Spawns>;
		array<CE_Items> matchingItems = new array<CE_Items>;
		
		foreach (CE_Items item : GetGeneratedItemData())
		{	
			foreach (CE_Spawns spawns : GetGeneratedSpawnData())
			{	
				if (item.Tiers & spawns.Tier &&
				item.Usages & spawns.Usage &&
				item.Category & spawns.Category)
				{
					if (!matchingSpawns.Contains(spawns))
					{
						matchingSpawns.Insert(spawns);
					}
					
					if (!matchingItems.Contains(item))
					{
						matchingItems.Insert(item);
					}
				}
			}
		}

		if (matchingSpawns.Count() > 0 && matchingItems.Count() > 0)
		{
			foreach (CE_Items matchedItem : matchingItems)
			{
				foreach (CE_Spawns matchedSpawn : matchingSpawns)
				{
					// why I gotta check again, idk, but it works now
					if (matchedItem.Tiers & matchedSpawn.Tier &&
					matchedItem.Usages & matchedSpawn.Usage &&
					matchedItem.Category & matchedSpawn.Category)
					{
						matchedSpawn.Items.Insert(matchedItem);
					}
				}
			}
		}
		return matchingSpawns;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryToSpawnLoot()
	{
		if (!Replication.IsServer())
			return;
		
		m_aMatchedSpawns.Clear();
		
		m_aMatchedSpawns = DetermineItemPool();
		
		if (m_aMatchedSpawns.Count() == 0)
			return;
		
		CE_ItemData m_Item;
		Resource m_Resource;
		
		foreach (CE_Spawns spawns : m_aMatchedSpawns)
		{	
			CE_ItemSpawningComponent comp = spawns.Spawn;
			
			IEntity lootSpawn = comp.GetOwner();
		
			vector m_WorldTransform[4];
			lootSpawn.GetWorldTransform(m_WorldTransform);
			
			if (!comp.GetHasItemSpawned())
			{			
				if (!spawns.Items)
					continue;
				
				m_Item = GetRandomItem(spawns.Items);
				
				if(!m_Item)
					continue;
				
				//Print(m_Item);
				
				m_Resource = Resource.Load(m_Item.m_sPrefab);
				
				EntitySpawnParams params();
				m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.200;
				params.Transform = m_WorldTransform;
			
				IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
				
				newEnt.SetYawPitchRoll(m_Item.m_vItemRotation + lootSpawn.GetYawPitchRoll());
				SCR_EntityHelper.SnapToGround(newEnt);
				
				SetItemQuantity(newEnt, m_Item.m_iQuantityMinimum, m_Item.m_iQuantityMaximum);
				
				m_aSpawnedItems.Insert(m_Item);
				comp.SetItemSpawned(m_Item);
				
				lootSpawn.AddChild(newEnt, -1, EAddChildFlags.NONE);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected CE_ItemData GetRandomItem(array<CE_Items> itemArray)
	{
		if(itemArray.Count() == 0)
			return null;
		int count = itemArray.Count() - 1;
		int random = m_randomGen.RandIntInclusive(0, count);
		
		CE_Items item = itemArray[random];
		
		return item.Item;
    }
	
	//------------------------------------------------------------------------------------------------
	// Performs checks on the new entity to see if it has quantity min and max set above 0, then determines which item type it is to then set quantity of the item (I.E. ammo count in a magazine)
	protected void SetItemQuantity(IEntity ent, int quantMin, int quantMax)
	{
		ResourceName prefabName = ent.GetPrefabData().GetPrefabName();
		
		float quantity;
		
		if (quantMin > 0 && quantMax > 0)
		{
			float randomFloat = Math.RandomIntInclusive(quantMin, quantMax) / 100; // convert it to a decimal to multiplied later on
			
			quantity = Math.Round(randomFloat * 10) / 10;
		}
		else
			return;
		
		
		if (prefabName.Contains("Magazine_") || prefabName.Contains("Box_"))
		{
			MagazineComponent magComp = MagazineComponent.Cast(ent.FindComponent(MagazineComponent));
			
			magComp.SetAmmoCount(magComp.GetMaxAmmoCount() * quantity);
		}
		else if (prefabName.Contains("Jerrycan_"))
		{
			FuelManagerComponent fuelManager = FuelManagerComponent.Cast(ent.FindComponent(FuelManagerComponent));
			
			array<BaseFuelNode> outNodes = {};
			
			fuelManager.GetFuelNodesList(outNodes);
			
			foreach (BaseFuelNode fuelNode : outNodes)
			{
				fuelNode.SetFuel(fuelNode.GetMaxFuel() * quantity);
			}
		}
		/*
		else if (prefabName.Contains("Canteen_"))
		{
			// eventually do this for canteens containing water for metabolism
		}
		else if (prefabName.Contains("Food_"))
		{
			// eventually do this for food items (I.E. canned items)
		}
		*/
		else
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_ItemData> GetSpawnedItems()
	{
		return m_aSpawnedItems;
	}

	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	void Register(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(notnull CE_ItemSpawningComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		Enable(false);
	}
}

class CE_Items
{
	CE_ItemData Item;
	CE_ELootTier Tiers;
	CE_ELootUsage Usages;
	CE_ELootCategory Category;
	
	//------------------------------------------------------------------------------------------------
	void CE_Items(CE_ItemData item, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category)
	{
		Item = item;
		Tiers = tiers;
		Usages = usages;
		Category = category;
	}
}

class CE_Spawns
{
	CE_ItemSpawningComponent Spawn;
	CE_ELootTier Tier;
	CE_ELootUsage Usage;
	CE_ELootCategory Category;
	ref array<CE_Items> Items = {};
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawns(CE_ItemSpawningComponent spawn, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		Spawn = spawn;
		Tier = tier;
		Usage = usage;
		Category = category;
		Items = new array<CE_Items>;
	}
}