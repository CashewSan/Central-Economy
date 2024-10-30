class CE_ItemSpawningSystem : GameSystem
{
	private static const int						CHECK_INTERVAL				= 15; 											// how often the item spawning system will run, (in seconds)
	
	protected ref array<ref CE_ItemData>			m_aItems 					= new array<ref CE_ItemData>; 					// storage of initial item data from config
	protected ref array<ref CE_Items>				m_aItemsToSpawn 				= new array<ref CE_Items>; 						// item data containing items that need to spawn
	protected ref array<ref CE_ItemData>			m_aSpawnedItems 				= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 	m_aComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref array<ref CE_Spawns> 			m_aComponentsForSpawn 		= new array<ref CE_Spawns>;  						// item spawning components in the world WITH NO LOOT
	protected ref array<CE_Spawns> 				m_aMatchedSpawns 			= new array<CE_Spawns>;							// spawns matched to item array
	protected ref array<ref CE_ItemData>			m_aItemsNotRestocked 			= new array<ref CE_ItemData>;						// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 			m_WorldValidationComponent;													// component added to world's gamemode for verification
	protected ref Resource 						m_itemDataConfig 			= BaseContainerTools.LoadContainer("{71359C0802F4ADA6}Configs/CE_ItemData.conf" /*"{F373DD039FDB3699}Configs/Testing/CE_ItemData_TEST.conf"*/); // config for item data
	protected ref CE_LootSpawningConfig			m_Config;
	protected CE_ELootTier						m_currentSpawnTier;															// current spawn tier that items are spawning at
	
	protected ref RandomGenerator 				m_randomGen 					= new RandomGenerator();
	
	protected bool 								b_HasGeneratedSpawnData		= false;
	protected bool 								b_HasGeneratedItemData		= false;
	protected bool 								b_WorldProcessed				= false;
	
	protected float 								m_fTimer;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
		
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;

		if (m_fTimer < CHECK_INTERVAL)
			return;

		m_fTimer = 0;
		
		if (b_WorldProcessed == true)
			ProcessData();
		else
			GetGame().GetCallqueue().CallLater(Init, 1000, false);
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
	protected void Init()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();

		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.GetWorldProcessed())
			{	
				b_WorldProcessed = true;
				
				BaseContainer m_Container = m_itemDataConfig.GetResource().ToBaseContainer();
				
				m_Config = CE_LootSpawningConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(m_Container));
				
				GenerateItemData();
				
				ProcessData();
			}
			else
			{
				Print("[Central Economy] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
				return;
			}
		}
		else
		{
			Print("[Central Economy] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateItemData()
	{
		if(!m_Config)
			return;
		
		if (m_Config.m_ItemData.Count() == 0)
			return;
		
		m_aItems.Clear();
		
		foreach(CE_ItemData item: m_Config.m_ItemData)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!item.m_sName || !item.m_sPrefab || !item.m_iRestock || !item.m_iNominal || !item.m_iMinimum || !item.m_ItemTiers || !item.m_iLifetime || !item.m_ItemUsages || !item.m_ItemCategory)
				continue;
			
			m_aItems.Insert(item);
		}
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
				break;
		}
		
		//Print(GetCurrentSpawnTier());
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
				if (comp.m_Tier & GetCurrentSpawnTier())
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
					
					CE_Spawns spawns = new CE_Spawns(comp, comp.m_Tier, comp.m_Usage, spawnCategory);
				
					m_aComponentsForSpawn.Insert(spawns);
				}
			}
		}
		SetHasGeneratedSpawnData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedSpawnData(bool meow)
	{
		b_HasGeneratedSpawnData = meow;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedSpawnData()
	{
		return b_HasGeneratedSpawnData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Spawns> GetGeneratedSpawnData()
	{
		return m_aComponentsForSpawn;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateItems()
	{	
		m_aItemsToSpawn.Clear();
		
		foreach(CE_ItemData item: m_aItems)
		{
			if (item.m_ItemTiers & GetCurrentSpawnTier())
			{
				int itemTargetCount = DetermineItemTargetCount(item, item.m_iMinimum, item.m_iNominal);
				
				CE_Items items = new CE_Items(item, item.m_ItemTiers, item.m_ItemUsages, item.m_ItemCategory);
				
				if (!m_aItemsToSpawn.Contains(items))
				{
					for (int i = 0; i < Math.ClampInt(itemTargetCount, item.m_iMinimum, item.m_iNominal); i++) // maybe this solves the issue of too many item spawns?
					{						
						m_aItemsToSpawn.Insert(items);
					}
				}
			}
		}
		
		SetHasGeneratedItemData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedItemData(bool meow)
	{
		b_HasGeneratedItemData = meow;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedItemData()
	{
		return b_HasGeneratedItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Items> GetGeneratedItemData()
	{
		return m_aItemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal) // double-check over all of this, items are spawning more than nominal
	{
		int itemTargetCount;
		int itemsNotRestocked = 0;
		
		if (GetSpawnedItems().Count() > 0)
		{
			int itemCount = 0;
			
			foreach (CE_ItemData spawnedItem : GetSpawnedItems())
			{
				string name = spawnedItem.m_sName;
				
				if (name == item.m_sName)
				{
					itemCount = itemCount + 1;
				}
			}
			
			int spawnCount = DetermineAvailableSpawnCount(item);
			
			if (itemCount >= nominal || spawnCount == 0)
			{
				itemTargetCount = 0; // no additional spawns of item needed
			}
			else
			{
				itemTargetCount = Math.ClampInt(itemCount + spawnCount, minimum, nominal);
			}
		}
		else
		{
			itemTargetCount = nominal;
		}
		
		
		if (GetItemsNotRestocked().Contains(item))
		{
			foreach (CE_ItemData itemNotRestocked : GetItemsNotRestocked())
			{
				itemsNotRestocked = itemsNotRestocked + 1;
			}
		}
		
		if (itemTargetCount == 0)
		{
			return itemTargetCount;
		}
		else
		{
			return Math.ClampInt(itemTargetCount - itemsNotRestocked, 0, itemTargetCount);
		}
		
		
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