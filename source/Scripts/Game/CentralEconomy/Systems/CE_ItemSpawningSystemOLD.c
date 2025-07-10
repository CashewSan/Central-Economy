/*
class CE_ItemSpawningSystem : GameSystem
{
	protected ref array<ref CE_ItemData>					m_aItemData						= new array<ref CE_ItemData>;						// CE_ItemData array, item data gets inserted from config item data
	protected ref array<ref CE_ItemData>					m_aSpawnedItems 					= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref array<CE_SearchableContainerComponent> 	m_aContainerComponents 			= new array<CE_SearchableContainerComponent>; 		// initial pull of ALL searchable container components in the world
	protected ref array<CE_ItemSpawningComponent> 			m_aComponentsWithoutItem 			= new array<CE_ItemSpawningComponent>; 			// Spawner components WITHOUT an item spawned
	protected ref array<CE_ItemSpawningComponent> 			m_aComponentsWithItem 			= new array<CE_ItemSpawningComponent>; 			// Spawner components WITH an item spawned
	protected ref map<ref CE_ItemData, string>				m_aItemsNotRestockReady 			= new map<ref CE_ItemData, string>;				// items whose restocking timer has not ended
	protected ref array<ref CE_Item>						m_aItems							= new array<ref CE_Item>;							// CE_Item array, gets filled after item validation
	
	protected CE_WorldValidationComponent 					m_WorldValidationComponent;														// component added to world's gamemode for verification
	protected CE_ItemDataConfig							m_Config;
	protected ref RandomGenerator 						m_randomGen 						= new RandomGenerator();							// vanilla random generator

	protected bool 										m_bWorldProcessed				= false;											// has the world been processed?
	
	protected float 										m_fTimer							= 0;												// timer for spawning check interval
	protected float 										m_fContainerResetTimer			= 0;												// timer for spawner reset
	protected float										m_fCheckInterval					= 0; 											// how often the item spawning system will run (in seconds)
	protected const float									m_fSpawnerResetCheckInterval		= 10;											// how often the system will check for spawner reset
	protected const float									m_fContainerResetCheckInterval	= 10;											// how often the system will check for spawner reset
	
	protected int										m_iSpawnedItemCount				= 0;												// count for spawned items, used to track when check interval changes
	protected int										m_iSortedItemsLimit				= 2000;											// sorted items count limit (basically, how many items can a spawner potentially sort and choose from to spawn)
	
	//------------------------------------------------------------------------------------------------
	//! Calls DelayedInit() upon initialization of the system
	override void OnInit()
	{
		DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, currently set to start spawning process of an item every n seconds
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		if (!m_aSpawnerComponents.IsEmpty())
		{
			if (m_fCheckInterval == 0)
				return;
			else
			{
				m_fTimer += timeSlice;
				
				if (m_fTimer >= m_fCheckInterval)
				{
					m_fTimer = 0;
					
					if (m_bWorldProcessed && m_Config)
					{
						SelectSpawnerAndItem();
					}
					else
						GetGame().GetCallqueue().CallLater(DelayedInit, 100, false);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delayed initialization, sets the m_WorldValidationComponent, sets m_WorldProcessed as true if found, and starts the process of spawning
	protected void DelayedInit()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed())
			{	
				//Print("World processed!");
				
				m_bWorldProcessed = true;
				
				if (m_WorldValidationComponent.m_fItemSpawningFrequency)
					m_fCheckInterval = m_WorldValidationComponent.m_fItemSpawningFrequency;
				else
					m_fCheckInterval = 1.0;
				
				m_Config = m_WorldValidationComponent.GetItemDataConfig();
						
				if (m_Config)
				{
					m_aItemData.Clear();
					
					foreach (CE_ItemData itemData : m_Config.m_ItemData)
					{
						m_aItemData.Insert(itemData);
					}
					
					GetGame().GetCallqueue().CallLater(SetItemsArray, 200);
				}
			}
			else
			{
				Print("[CentralEconomy::CE_ItemSpawnSystem] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
				return;
			}
		}
		else
		{
			Print("[CentralEconomy::CE_ItemSpawnSystem] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the items array from universal config
	protected void SetItemsArray()
	{
		m_aItems = GenerateItemsFromUniversalConfig();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Selects a randomized spawner component to try to spawn an item at, and requests the randomized item to be spawned
	protected void SelectSpawnerAndItem()
	{
		int index;
		
		if (m_aComponentsWithoutItem.Count() > 1)
			index = m_randomGen.RandInt(0, m_aComponentsWithoutItem.Count());
		else if (m_aComponentsWithoutItem.Count() == 1)
			index = 0;
		else
			return;
		
		if (m_aComponentsWithoutItem.IsEmpty() || !m_aComponentsWithoutItem[index] || m_aComponentsWithoutItem[index] && m_aComponentsWithoutItem[index].HasItemSpawned())
			return;
		else
		{
			CE_ItemData item;
			
			if (m_aComponentsWithoutItem[index].HasSpawnerConfig())
				item = m_aComponentsWithoutItem[index].RequestItemToSpawnFromSpawnerConfig();
			else
				item = m_aComponentsWithoutItem[index].GetItemToSpawnFromUniversalConfig();
			
			if (item)
				TryToSpawnItem(m_aComponentsWithoutItem[index], item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the item
	void TryToSpawnItem(CE_ItemSpawningComponent spawner, CE_ItemData item)
	{
		if (!Replication.IsServer())
			return;
		
		if (spawner.HasItemSpawned())
		{		
			return;
		}
		else if (item)
		{
			//spawner.SetItemSpawned(item);
			
			IEntity spawnEntity = spawner.GetOwner();
			if (!spawnEntity)
				return;
		
			vector m_WorldTransform[4];
			spawnEntity.GetWorldTransform(m_WorldTransform);
			
			if (item.m_vItemRotation != vector.Zero)
				Math3D.AnglesToMatrix(item.m_vItemRotation, m_WorldTransform);
			
			EntitySpawnParams params();
			m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.025; // to not make item be in the ground
			params.Transform = m_WorldTransform;
			
			Resource m_Resource = Resource.Load(item.m_sPrefab);
			if (!m_Resource)
				return;
			
			IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
			if (!newEnt)
				return;
			
			
			//newEnt.SetYawPitchRoll(item.m_vItemRotation + spawnEntity.GetYawPitchRoll()); // broken as of 1.3
			//SCR_EntityHelper.SnapToGround(newEnt); // puts item halfway into ground
			
			SetItemQuantity(newEnt, item.m_iQuantityMinimum, item.m_iQuantityMaximum);
			
			GetSpawnedItems().Insert(item);
			//ResetStallTimer();
			SetSpawnedItemCount(GetSpawnedItemCount() + 1);
			
			GetComponentsWithItem().Insert(spawner);
			GetComponentsWithoutItem().RemoveItem(spawner);
			
			// if item is a vehicle, or at least categorized as one (vehicles don't have a hierarchy component, so adding as child doesn't work)
			if (item.m_ItemCategory == CE_ELootCategory.VEHICLES
			|| item.m_ItemCategory == CE_ELootCategory.VEHICLES_SEA
			|| item.m_ItemCategory == CE_ELootCategory.VEHICLES_AIR)
			{
				// Vehicle enter/leave event
		        EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(newEnt.FindComponent(EventHandlerManagerComponent));
		        if (eventHandler)
		        {
		           	eventHandler.RegisterScriptHandler("OnCompartmentEntered", this, spawner.OnVehicleActivated, true, false);
					eventHandler.RegisterScriptHandler("OnCompartmentLeft", this, spawner.OnVehicleDeactivated, true, false);
		        }
			}
			
			spawner.GetItemSpawnedInvoker().Invoke(newEnt, item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Performs checks on the new entity to see if it has quantity min and max set above 0, then determines which item type it is to then set quantity of the item (I.E. ammo count in a magazine)
	protected void SetItemQuantity(IEntity ent, int quantMin, int quantMax)
	{
		float quantity;
		
		if (quantMin > 0 && quantMax > 0)
		{
			float randomFloat = Math.RandomIntInclusive(quantMin, quantMax) / 100; // convert it to a decimal to multiplied later on
			
			quantity = Math.Round(randomFloat * 10) / 10;
		}
		else
			return;
		
		ResourceName prefabName = ent.GetPrefabData().GetPrefabName();
		
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
		else
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to generate items and calls to get a selected item
	CE_ItemData GetItemFromSpawnerConfig(array<ref CE_ItemData> itemDataArray, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		if (itemDataArray.IsEmpty())
		{
			return null;
		}
		
		array<ref CE_Item> items = GenerateItemsFromSpawnerConfig(itemDataArray, tier, usage, category);
		
		if (items.IsEmpty())
			return null;
		else
			return SelectItemFromSpawnerConfig(items);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to generate items and calls to get multiple selected items
	array<CE_ItemData> GetItemsFromContainerConfig(array<ref CE_ItemData> itemDataArray, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category, int minimum, int maximum)
	{
		if (itemDataArray.IsEmpty())
		{
			return null;
		}
		
		array<ref CE_Item> items = GenerateItemsFromSpawnerConfig(itemDataArray, tier, usage, category);
		
		if (items.IsEmpty())
			return null;
		else
		{
			return SelectItemsFromSpawnerConfig(items, minimum, maximum);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to generate items and calls to get a selected item
	CE_ItemData GetItemFromUniversalConfig(CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		array<ref CE_Item> sortedItems = {};
		
		int m_iSortedItemsCount = 0;
		
		bool isLimitReached = false;
		
		foreach (CE_Item item : m_aItems)
		{
			int index;
			
			if (m_aItems.Count() > 1)
				index = m_randomGen.RandInt(0, m_aItems.Count());
			else if (m_aItems.Count() == 1)
				index = 0;
			else
				continue;
			
			if (!m_aItems[index] || m_aItems[index] && !m_aItems[index].Tiers || m_aItems[index] && !m_aItems[index].Usages || m_aItems[index] && !m_aItems[index].Category)
				continue;
			
			if (m_aItems[index].Tiers & tier 
			&& m_aItems[index].Usages & usage 
			&& m_aItems[index].Category & category)
			{
				int itemTargetCount = DetermineItemTargetCountFromItem(m_aItems[index]);
				
				//Print("Item: " + m_aItems[index].Item.m_sName + ", TargetCount: " + itemTargetCount);
				
				if (itemTargetCount > 0)
				{
					for (int e = 0; e < itemTargetCount; e++)
					{
						sortedItems.Insert(m_aItems[index]);
						
						m_iSortedItemsCount = m_iSortedItemsCount + 1;
						
						if (m_iSortedItemsCount >= m_iSortedItemsLimit)
						{
							//Print("Breaking");
							m_iSortedItemsCount = 0;
							isLimitReached = true;
							break;
						}
					}
					
				}
			}
			if (isLimitReached)
				break;
		}
		
		if (sortedItems.IsEmpty())
			return null;
		else
			return SelectItemFromUniversalConfig(sortedItems);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to generate items and calls to get a selected item
	array<CE_ItemData> GetItemsFromUniversalConfig(CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category, int minimum, int maximum)
	{
		array<ref CE_Item> sortedItems = {};
		
		int m_iSortedItemsCount = 0;
		
		bool isLimitReached = false;
		
		foreach (CE_Item item : m_aItems)
		{
			int index;
			
			if (m_aItems.Count() > 1)
				index = m_randomGen.RandInt(0, m_aItems.Count());
			else if (m_aItems.Count() == 1)
				index = 0;
			else
				continue;
			
			if (!m_aItems[index] || m_aItems[index] && !m_aItems[index].Tiers || m_aItems[index] && !m_aItems[index].Usages || m_aItems[index] && !m_aItems[index].Category)
				continue;
			
			if (m_aItems[index].Tiers & tier 
			&& m_aItems[index].Usages & usage 
			&& m_aItems[index].Category & category)
			{
				int itemTargetCount = DetermineItemTargetCountFromItem(m_aItems[index]);
				
				//Print("Item: " + m_aItems[index].Item.m_sName + ", TargetCount: " + itemTargetCount);
				
				if (itemTargetCount > 0)
				{
					for (int e = 0; e < itemTargetCount; e++)
					{
						sortedItems.Insert(m_aItems[index]);
						
						m_iSortedItemsCount = m_iSortedItemsCount + 1;
						
						if (m_iSortedItemsCount >= m_iSortedItemsLimit)
						{
							//Print("Breaking");
							m_iSortedItemsCount = 0;
							isLimitReached = true;
							break;
						}
					}
					
				}
			}
			if (isLimitReached)
				break;
		}
		
		if (sortedItems.IsEmpty())
			return null;
		else
			return SelectItemsFromUniversalConfig(sortedItems, minimum, maximum);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Generates items and separates item data that has necessary information and corresponds to spawner component attributes
	protected array<ref CE_Item> GenerateItemsFromSpawnerConfig(array<ref CE_ItemData> items, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		Print(SCR_Enum.GetEnumName(CE_ELootUsage, usage));
		Print(SCR_Enum.GetEnumName(CE_ELootCategory, category));
		
		array<ref CE_Item> itemsToSpawn = {};
		
		foreach (CE_ItemData itemData : items)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!itemData.m_sName 
			|| !itemData.m_sPrefab 
			|| !itemData.m_iNominal 
			|| !itemData.m_iMinimum 
			|| !itemData.m_iLifetime 
			|| !itemData.m_iRestock 
			|| !itemData.m_ItemCategory 
			|| !itemData.m_ItemUsages 
			|| !itemData.m_ItemTiers)
			{
				Print("[CentralEconomy::CE_ItemSpawnSystem] " + itemData.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			else if (itemData.m_ItemTiers & tier 
			&& itemData.m_ItemUsages & usage 
			&& itemData.m_ItemCategory & category) // has matching tier, usage, and category with spawner component
			{
				int itemTargetCount = DetermineItemTargetCountFromItemData(itemData, itemData.m_iMinimum, itemData.m_iNominal);
				
				//Print("Item: " + itemData.m_sName + ", TargetCount: " + itemTargetCount);
				
				if (itemTargetCount != 0)
				{
					CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
					
					if (!itemsToSpawn.Contains(item))
					{
						for (int i = 0; i < Math.ClampInt(itemTargetCount, 0, itemData.m_iNominal); i++)
						{
							itemsToSpawn.Insert(item);
						}
					}
				}
			}
		}
		
		return itemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Generates items and separates item data that has necessary information and corresponds to spawner component attributes
	protected array<ref CE_Item> GenerateItemsFromUniversalConfig()
	{
		array<ref CE_Item> itemsToSpawn = {};
		
		foreach (CE_ItemData itemData : m_aItemData)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!itemData.m_sName 
			|| !itemData.m_sPrefab 
			|| !itemData.m_iNominal 
			|| !itemData.m_iMinimum 
			|| !itemData.m_iLifetime 
			|| !itemData.m_iRestock 
			|| !itemData.m_ItemCategory 
			|| !itemData.m_ItemUsages 
			|| !itemData.m_ItemTiers)
			{
				Print("[CentralEconomy::CE_ItemSpawnSystem] " + itemData.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			else
			{	
				CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
					
				if (!itemsToSpawn.Contains(item))
				{
					itemsToSpawn.Insert(item);
				}
			}
		}
		
		return itemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determines the item count of based of nominal, minimum, already spawned items, and those not restock ready
	protected int DetermineItemTargetCountFromItemData(CE_ItemData item, int minimum, int nominal)
	{
		//map<ref CE_ItemData, string> itemsNotRestockReady = GetItemsNotRestockReady();
		//array<ref CE_ItemData> spawnedItems = GetSpawnedItems();
		
		int itemCount = 0;
		
		// for every item spawned in the world
		foreach (CE_ItemData spawnedItem : GetSpawnedItems())
		{
			string name = spawnedItem.m_sName;
			
			// increase item count if there is a name match
			if (name == item.m_sName)
			{
				itemCount = itemCount + 1;
			}
		}
		
		int targetCount = Math.ClampInt(m_randomGen.RandIntInclusive(minimum, nominal) - itemCount, 0, nominal);
		
		for (int i = 0; i < GetItemsNotRestockReady().Count(); i++)
		{
			if (GetItemsNotRestockReady().GetKeyByValue(item.m_sName))
				targetCount = Math.ClampInt(targetCount - 1, 0, nominal);
		}
		
		return Math.ClampInt(targetCount, 0, nominal);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determines the item count of based of nominal, minimum, already spawned items, and those not restock ready
	protected int DetermineItemTargetCountFromItem(CE_Item item)
	{
		//map<ref CE_ItemData, string> itemsNotRestockReady = GetItemsNotRestockReady();
		//array<ref CE_ItemData> spawnedItems = GetSpawnedItems();
		
		int itemCount = 0;
		
		CE_ItemData itemData = item.Item;
		
		// for every item spawned in the world
		foreach (CE_ItemData spawnedItem : GetSpawnedItems())
		{
			string name = spawnedItem.m_sName;
			
			// increase item count if there is a name match
			if (name == itemData.m_sName)
			{
				itemCount = itemCount + 1;
			}
		}
		
		int targetCount = Math.ClampInt(m_randomGen.RandIntInclusive(itemData.m_iMinimum, itemData.m_iNominal) - itemCount, 0, itemData.m_iNominal);
		
		for (int i = 0; i < GetItemsNotRestockReady().Count(); i++)
		{
			if (GetItemsNotRestockReady().GetKeyByValue(itemData.m_sName))
				targetCount = Math.ClampInt(targetCount - 1, 0, itemData.m_iNominal);
		}
		
		return Math.ClampInt(targetCount, 0, itemData.m_iNominal);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Selects item from itemArray, chooses it based on probability algorithm
	protected CE_ItemData SelectItemFromSpawnerConfig(array<ref CE_Item> itemArray)
	{
		if(itemArray.IsEmpty())
			return null;
		
		CE_ItemData itemData;
		
		//Print(itemArray.Count());
		
		map<CE_Item, int> counts = new map<CE_Item, int>;
		
		int probabilityTotal = 0;
		
		for (int i = 0; i < itemArray.Count(); i++)
		{
			if (itemArray.Contains(itemArray[i]))
			{
				counts[itemArray[i]] = counts[itemArray[i]] + 1;
			}
			else
			{	
				counts[itemArray[i]] = 1;
			}
			
			probabilityTotal += counts.Get(itemArray[i]);
		}
		
		foreach (CE_Item item : itemArray)
		{
			if (Probability(counts.Get(item), probabilityTotal))
			{
				itemData = item.Item;
				break;
			}
		}
		
		//Print("Probability Total: " + probabilityTotal);
		
		return itemData;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Selects multiple items from itemArray, chooses it based on probability algorithm
	protected array<CE_ItemData> SelectItemsFromSpawnerConfig(array<ref CE_Item> itemArray, int minimum, int maximum)
	{
		if(itemArray.IsEmpty())
			return null;
		
		array<CE_ItemData> itemDatas = {};
		
		//Print(itemArray.Count());
		
		int randomCount = m_randomGen.RandIntInclusive(minimum, maximum);
		
		for (int a = 0; a < randomCount; a++)
		{
			map<CE_Item, int> counts = new map<CE_Item, int>;
			
			int probabilityTotal = 0;
			
			for (int i = 0; i < itemArray.Count(); i++)
			{
				if (itemArray.Contains(itemArray[i]))
				{
					counts[itemArray[i]] = counts[itemArray[i]] + 1;
				}
				else
				{	
					counts[itemArray[i]] = 1;
				}
				
				probabilityTotal += counts.Get(itemArray[i]);
			}
			
			foreach (CE_Item item : itemArray)
			{
				if (Probability(counts.Get(item), probabilityTotal))
				{
					itemDatas.Insert(item.Item);
					break;
				}
			}
		}
		
		//Print("Probability Total: " + probabilityTotal);
		
		return itemDatas;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Selects item from itemArray, chooses it based on probability algorithm
	protected CE_ItemData SelectItemFromUniversalConfig(array<ref CE_Item> itemArray)
	{
		if(itemArray.IsEmpty())
			return null;

		//Print("Matching Item Count: " + itemArray.Count());
		
		map<CE_Item, int> counts = new map<CE_Item, int>;
		
		int probabilityTotal = 0;
		
		for (int i = 0; i < itemArray.Count(); i++)
		{
			if (itemArray.Contains(itemArray[i]))
			{
				counts[itemArray[i]] = counts[itemArray[i]] + 1;
			}
			else
			{	
				counts[itemArray[i]] = 1;
			}
			
			probabilityTotal += counts.Get(itemArray[i]);
		}
		
		CE_ItemData itemData;
		
		foreach (CE_Item item : itemArray)
		{
			if (Probability(counts.Get(item), probabilityTotal))
			{
				itemData = item.Item;
				break;
			}
		}
		
		//Print("Probability Total: " + probabilityTotal);
		
		return itemData;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Selects item from itemArray, chooses it based on probability algorithm
	protected array<CE_ItemData> SelectItemsFromUniversalConfig(array<ref CE_Item> itemArray, int minimum, int maximum)
	{
		if(itemArray.IsEmpty())
			return null;

		array<CE_ItemData> itemDatas = {};
		
		int randomCount = m_randomGen.RandIntInclusive(minimum, maximum);
		
		for (int a = 0; a < randomCount; a++)
		{
			map<CE_Item, int> counts = new map<CE_Item, int>;
			
			int probabilityTotal = 0;
			
			for (int i = 0; i < itemArray.Count(); i++)
			{
				if (itemArray.Contains(itemArray[i]))
				{
					counts[itemArray[i]] = counts[itemArray[i]] + 1;
				}
				else
				{	
					counts[itemArray[i]] = 1;
				}
				
				probabilityTotal += counts.Get(itemArray[i]);
			}
			
			
			foreach (CE_Item item : itemArray)
			{
				if (Probability(counts.Get(item), probabilityTotal))
				{
					itemDatas.Insert(item.Item);
					break;
				}
			}
		}
		
		//Print("Probability Total: " + probabilityTotal);
		
		return itemDatas;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Chooses a randomized number between 0 and the probabilityTotal to the select and item it it's value is greater than said randomized number
	protected bool Probability(float probability, float probabilityTotal)
	{
		//Print("Probability: " + probability);
		
		int random = m_randomGen.RandInt(0, probabilityTotal);
		
		return probability >= random;
	}
	
	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	//! Gets the system instance
	static CE_ItemSpawningSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! Registers spawning component
	void RegisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aSpawnerComponents.Contains(component))
		{
			m_aSpawnerComponents.Insert(component);
			
			if (!component.HasItemSpawned())
				m_aComponentsWithoutItem.Insert(component);
		}
			
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawning component
	void UnregisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		m_aSpawnerComponents.RemoveItem(component);
		
		if (m_aSpawnerComponents.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers searchable container component
	void RegisterContainer(notnull CE_SearchableContainerComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aContainerComponents.Contains(component))
		{
			m_aContainerComponents.Insert(component);
		}
			
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters searchable container component
	void UnregisterContainer(notnull CE_SearchableContainerComponent component)
	{
		m_aContainerComponents.RemoveItem(component);
		
		if (m_aContainerComponents.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawned item array
	array<ref CE_ItemData> GetSpawnedItems()
	{
		return m_aSpawnedItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawner array, but only of spawners WITHOUT an item
	array<CE_ItemSpawningComponent> GetComponentsWithoutItem()
	{
		return m_aComponentsWithoutItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawner array, but only of spawners WITH an item
	array<CE_ItemSpawningComponent> GetComponentsWithItem()
	{
		return m_aComponentsWithItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the array for items that are NOT restock ready
	map<ref CE_ItemData, string> GetItemsNotRestockReady()
	{
		return m_aItemsNotRestockReady;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the spawn item count
	int GetSpawnedItemCount()
	{
		return m_iSpawnedItemCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the spawn item count
	void SetSpawnedItemCount(int count)
	{
		m_iSpawnedItemCount = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets all components registered to system
	array<CE_ItemSpawningComponent> GetComponents()
	{
		return m_aSpawnerComponents;
	}
}

class CE_Item
{
	CE_ItemData Item;
	CE_ELootTier Tiers;
	CE_ELootUsage Usages;
	CE_ELootCategory Category;
	
	//------------------------------------------------------------------------------------------------
	void CE_Item(CE_ItemData item, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category)
	{
		Item = item;
		Tiers = tiers;
		Usages = usages;
		Category = category;
	}
}