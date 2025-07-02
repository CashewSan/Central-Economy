class CE_ItemSpawningSystem : GameSystem
{
	//protected ref array<ref CE_ItemData>					m_aItemData						= new array<ref CE_ItemData>;						// CE_ItemData array, item data gets inserted from config item data
	protected ref array<ref CE_ItemData>					m_aSpawnedItems 					= new array<ref CE_ItemData>; 					// items that have spawned in the world
	//protected ref array<CE_ItemSpawningComponent> 			m_aSpawners 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	//protected ref array<CE_SearchableContainerComponent> 	m_aContainers 			= new array<CE_SearchableContainerComponent>; 		// initial pull of ALL searchable container components in the world
	//protected ref array<CE_ItemSpawningComponent> 			m_aComponentsWithoutItem 			= new array<CE_ItemSpawningComponent>; 			// Spawner components WITHOUT an item spawned
	//protected ref array<CE_ItemSpawningComponent> 			m_aComponentsWithItem 			= new array<CE_ItemSpawningComponent>; 			// Spawner components WITH an item spawned
	protected ref map<ref CE_ItemData, string>				m_aItemsNotRestockReady 			= new map<ref CE_ItemData, string>;				// items whose restocking timer has not ended
	//protected ref array<ref CE_Item>						m_aItems							= new array<ref CE_Item>;							// CE_Item array, gets filled after item validation
	
	//protected CE_WorldValidationComponent 					m_WorldValidationComponent;														// component added to world's gamemode for verification
	//protected CE_ItemDataConfig							m_Config;
	//protected ref RandomGenerator 						m_randomGen 						= new RandomGenerator();							// vanilla random generator

	protected bool 										m_bWorldProcessed				= false;											// has the world been processed?
	//protected bool 										m_bInitiallyRan					= false;											// has the spawn system had it's initial spawning in preload?
	
	//protected float 										m_fTimer							= 0;												// timer for spawning check interval
	//protected float 										m_fContainerResetTimer			= 0;												// timer for spawner reset
	//protected float										m_fCheckInterval					= 0; 											// how often the item spawning system will run (in seconds)
	protected const float									m_fSpawnerResetCheckInterval		= 10;											// how often the system will check for spawner reset
	protected const float									m_fContainerResetCheckInterval	= 10;											// how often the system will check for spawner reset
	
	protected int										m_iSpawnedItemCount				= 0;												// count for spawned items, used to track when check interval changes
	protected int										m_iSortedItemsLimit				= 2000;											// sorted items count limit (basically, how many items can a spawner potentially sort and choose from to spawn)
	
	
	
	
	
	// New variables
	protected ref map<CE_ItemSpawningComponent, bool> 		m_aSpawners		 				= new map<CE_ItemSpawningComponent, bool>; 		// Spawners that have been processed
	protected ref array<CE_SearchableContainerComponent> 	m_aContainers 					= new array<CE_SearchableContainerComponent>; 		// ALL registered searchable container components in the world
	
	protected ref map<ref CE_Item, int>					m_aItems 						= new map<ref CE_Item, int>;						// Processed CE_ItemData into CE_Item with count of available
	
	protected CE_ItemDataConfig							m_Config;																		// config file containing CE_ItemData
	protected CE_WorldValidationComponent 					m_WorldValidationComponent;
	protected ref RandomGenerator 						m_randomGen 						= new RandomGenerator();							// vanilla random generator
	
	protected float 										m_fTimer							= 0;												// timer for spawning check interval
	protected float										m_fProcessSpawnersInterval		= 2; 											// interval (in seconds) for proccessing spawners
	
	protected bool										m_bInitiallyRan					= false;
	
	//------------------------------------------------------------------------------------------------
	//! 
	override void OnInit()
	{
		if (GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if (m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed())
			{
				m_Config = m_WorldValidationComponent.GetItemDataConfig();
				
				Print("ItemData Count: " + m_Config.m_ItemData.Count());
				
				m_aItems = ProcessItemData(m_Config);
				
				GetGame().GetCallqueue().CallLater(InitialSpawnRun, 1000); // to wait for m_aItems to process
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
	//!
	protected void InitialSpawnRun()
	{
		if (!m_aItems.IsEmpty())
		{
			array<CE_ItemSpawningComponent> spawners = ProcessSpawners();
			
			Print("Spawners Count: " + m_aSpawners.Count());
			Print("Processed Spawners Count: " + spawners.Count());
			
			int runCount = 0;
			int spawnersHalfCount = spawners.Count() * 0.5;
			
			for (int i = 0; i < spawnersHalfCount; i++)
			{
				if (spawners.IsEmpty())
					return;
				
				int randomIndex = m_randomGen.RandInt(0, spawners.Count());
				if (!randomIndex)
					return;
				
				CE_Item item = SelectItem(m_aItems, spawners[randomIndex]);
				if (!item)
					return;
				
				if (TryToSpawnItem(spawners[randomIndex], item))
					spawners.Remove(randomIndex);
				
				Print("Spawners: " + spawners.Count());
				
				runCount++;
				
				Print(runCount);
				
				if (runCount >= spawnersHalfCount)
				{
					m_bInitiallyRan = true;
					Print("Reached Run Count");
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	override event protected void OnUpdate(ESystemPoint point)
	{
		if (m_bInitiallyRan)
		{
			float timeSlice = GetWorld().GetFixedTimeSlice();
			
			if (!m_aItems.IsEmpty())
			{
				m_fTimer += timeSlice;
				
				if (m_fTimer >= m_fProcessSpawnersInterval)
				{
					m_fTimer = 0;
					
					CE_ItemSpawningComponent spawner = SelectSpawner(ProcessSpawners());
					
					if (spawner)
					{
						CE_Item item = SelectItem(m_aItems, spawner);
						
						if (item)
							TryToSpawnItem(spawner, item);
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	map<ref CE_Item, int> ProcessItemData(CE_ItemDataConfig config)
	{	
		map<ref CE_Item, int>	 itemsProcessed = new map<ref CE_Item, int>;
		array<ref CE_ItemData> itemDataToBeProcessed = {};
		
		foreach (CE_ItemData itemData : config.m_ItemData)
		{
			itemDataToBeProcessed.Insert(itemData)
		}
		
		Print("ItemDataToBeProcessed Count: " + itemDataToBeProcessed.Count());
		
		int itemCount = itemDataToBeProcessed.Count();
		
		for (int i = 0, maxCount = Math.Min(itemCount, config.m_ItemData.Count()); i < maxCount; i++)
		{
			CE_ItemData itemData = itemDataToBeProcessed[0];
			
			if (!itemData.m_sName 
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
					
				if (!itemsProcessed.Contains(item))
				{
					itemsProcessed.Insert(item, itemData.m_iNominal);
				}
			}
			
			itemDataToBeProcessed.Remove(0);
			itemCount--;
		}
		
		Print("Item Count: " + itemsProcessed.Count());
		
		return itemsProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected array<CE_ItemSpawningComponent> ProcessSpawners()
	{
		array<CE_ItemSpawningComponent> spawners = {};
		
		for (int i = 0; i < m_aSpawners.Count(); i++)
		{
			if (m_aSpawners.GetKeyByValue(false))
			{
				spawners.Insert(m_aSpawners.GetKey(i));
			}
		}
		
		return spawners;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected CE_ItemSpawningComponent SelectSpawner(array<CE_ItemSpawningComponent> spawnersArray)
	{
		if (spawnersArray.IsEmpty())
			return null;
		
		int index = m_randomGen.RandInt(0, spawnersArray.Count());
		
		Print("Index: " + index);
		
		return spawnersArray[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected CE_Item SelectItem(map<ref CE_Item, int> itemsArray, CE_ItemSpawningComponent spawner)
	{
		CE_Item item;
		
		//int index = m_randomGen.RandInt(0, itemsArray.Count());
		
		for (int i = 0; i < itemsArray.Count(); i = m_randomGen.RandInt(0, itemsArray.Count()))
		{
			CE_Item itemKey = itemsArray.GetKey(i);
			
			if (!itemKey
			|| itemKey && !itemKey.Tiers 
			|| itemKey && !itemKey.Usages 
			|| itemKey && !itemKey.Category)
			{
				itemsArray.Remove(itemKey);
				continue;
			}
			
			int itemCount = itemsArray.Get(itemKey);
			
			if (itemKey.Tiers & spawner.GetSpawnerTier() 
			&& itemKey.Usages & spawner.GetSpawnerUsage() 
			&& itemKey.Category & spawner.m_Categories
			&& itemCount > 0)
			{
				item = itemKey;
				break;
			}
			else
			{
				itemsArray.Remove(itemKey);
				continue;
			}
		}
		
		return item;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the item, returns true if item spawned properly
	protected bool TryToSpawnItem(CE_ItemSpawningComponent spawner, CE_Item item)
	{
		if (!Replication.IsServer())
			return false;
		
		if (item)
		{
			CE_ItemData itemData = item.Item;
			if (!itemData)
				return false;
			
			IEntity spawnEntity = spawner.GetOwner();
			if (!spawnEntity)
				return false;
		
			vector m_WorldTransform[4];
			spawnEntity.GetWorldTransform(m_WorldTransform);
			
			if (itemData.m_vItemRotation != vector.Zero)
				Math3D.AnglesToMatrix(itemData.m_vItemRotation, m_WorldTransform);
			
			EntitySpawnParams params();
			m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.025; // to not make item be in the ground
			params.Transform = m_WorldTransform;
			
			Resource m_Resource = Resource.Load(itemData.m_sPrefab);
			if (!m_Resource)
				return false;
			
			IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
			if (!newEnt)
				return false;
			
			
			//newEnt.SetYawPitchRoll(item.m_vItemRotation + spawnEntity.GetYawPitchRoll()); // broken as of 1.3
			//SCR_EntityHelper.SnapToGround(newEnt); // puts item halfway into ground
			
			SetItemQuantity(newEnt, itemData.m_iQuantityMinimum, itemData.m_iQuantityMaximum);
			
			//GetSpawnedItems().Insert(itemData);
			//ResetStallTimer();
			//SetSpawnedItemCount(GetSpawnedItemCount() + 1);
			
			//GetComponentsWithItem().Insert(spawner);
			//GetComponentsWithoutItem().RemoveItem(spawner);
			
			// if item is a vehicle, or at least categorized as one (vehicles don't have a hierarchy component, so adding as child doesn't work)
			if (itemData.m_ItemCategory == CE_ELootCategory.VEHICLES
			|| itemData.m_ItemCategory == CE_ELootCategory.VEHICLES_SEA
			|| itemData.m_ItemCategory == CE_ELootCategory.VEHICLES_AIR)
			{
				// Vehicle enter/leave event
		        EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(newEnt.FindComponent(EventHandlerManagerComponent));
		        if (eventHandler)
		        {
		           	eventHandler.RegisterScriptHandler("OnCompartmentEntered", this, spawner.OnVehicleActivated, true, false);
					eventHandler.RegisterScriptHandler("OnCompartmentLeft", this, spawner.OnVehicleDeactivated, true, false);
		        }
			}
			
			m_aSpawners.Set(spawner, true);
			
			spawner.GetItemSpawnedInvoker().Invoke(newEnt, itemData);
			
			return true;
		}
		else
			return false;
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

	int meow = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Registers spawning component
	void RegisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aSpawners.Contains(component))
		{
			meow++;
			m_aSpawners.Insert(component, false);
		}
		
		Print(meow);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawning component
	void UnregisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		m_aSpawners.Remove(component);
		
		if (m_aSpawners.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers searchable container component
	void RegisterContainer(notnull CE_SearchableContainerComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aContainers.Contains(component))
		{
			m_aContainers.Insert(component);
		}
			
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters searchable container component
	void UnregisterContainer(notnull CE_SearchableContainerComponent component)
	{
		m_aContainers.RemoveItem(component);
		
		if (m_aContainers.IsEmpty())
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