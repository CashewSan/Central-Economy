class CE_ItemSpawningSystem : GameSystem
{
	/* 
		To keep this brief, this is the brains of CentralEconomy. Handles all item & spawner processing, and then spawning items to spawners
	
		Enjoy!
			- Cashew
	*/
	
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents		 		= new array<CE_ItemSpawningComponent>; 			// ALL registered spawner components in the world
	protected ref array<ref CE_Spawner>					m_aSpawners						= new array<ref CE_Spawner>;						// processed CE_ItemSpawningComponent into CE_Spawner
	protected ref array<CE_SearchableContainerComponent> 	m_aContainerComponents 			= new array<CE_SearchableContainerComponent>; 		// ALL registered searchable container components in the world
	
	[RplProp()]
	protected ref array<ref CE_Item>						m_aItems 						= new array<ref CE_Item>;							// processed CE_ItemData into CE_Item
	
	protected ref array<CE_UsageTriggerArea>				m_aUsageAreas					= new array<CE_UsageTriggerArea>;					// usage areas registered to the system
	protected ref array<CE_TierTriggerArea>				m_aTierAreas						= new array<CE_TierTriggerArea>;					// tier areas registered to the system
	
	protected ref RandomGenerator 						m_RandomGen						= new RandomGenerator();							// vanilla random generator
	
	protected float 										m_fTimer							= 0;												// timer for spawning check interval
	protected float										m_fItemSpawningFrequency			= 0; 											// frequency (in seconds) that an item will spawn
	protected float										m_fItemSpawningRatio				= 0;												// ratio of items the system will aim to spawn compared to spawners
	
	protected int										m_iItemCount						= 0;												// count of items currently on a spawner
	protected int										m_iSpawnerRatioCount				= 0;												// count of spawners multiplied by the m_fItemSpawningRatio
	protected int										m_iUsageAreasQueried				= 0;												// count of usage areas queried
	protected int										m_iTierAreasQueried				= 0;												// count of tier areas queried
	
	protected bool										m_bHaveAreasQueried				= false;											// have the usage and tier areas finished querying?
	protected bool										m_bInitiallyRan					= false;											// has the system finished it's initial spawning phase?
	
	//------------------------------------------------------------------------------------------------
	//! On system start
	override void OnStarted()
	{
		super.OnStarted();
		
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
		Print("meowmeowmeow");
		
		CE_WorldValidationComponent m_WorldValidationComponent;
		
		if (GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if (m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed())
			{
				m_fItemSpawningFrequency = m_WorldValidationComponent.m_fItemSpawningFrequency;
				m_fItemSpawningRatio = m_WorldValidationComponent.m_fItemSpawningRatio;
				
				CE_ItemDataConfig m_Config = m_WorldValidationComponent.GetItemDataConfig();
				
				m_aItems = ProcessItemData(m_Config);
				
				Replication.BumpMe();
			}
			else
			{
				Print("[CentralEconomy::CE_ItemSpawningSystem] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
				return;
			}
		}
		else
		{
			Print("[CentralEconomy::CE_ItemSpawningSystem] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles the initial spawning phase, is called when all usage and tier areas have been queried
	protected void InitialSpawningPhase()
	{
		if (m_aItems && !m_aItems.IsEmpty())
		{
			m_bInitiallyRan = true;
			
			array<ref CE_Spawner> processedSpawnersArray = {};
			
			foreach (CE_Spawner spawner : ProcessSpawners())
			{
				processedSpawnersArray.Insert(spawner);
			}
			
			m_iSpawnerRatioCount = Math.Round(processedSpawnersArray.Count() * m_fItemSpawningRatio);
			int runCount = 0;
			int failCount = 0;
			
			while (runCount < m_iSpawnerRatioCount && failCount < processedSpawnersArray.Count())
			{
				CE_Spawner spawner = SelectSpawner(processedSpawnersArray);
				
				if (spawner)
				{
					array<ref CE_Item> items = {};
					
					CE_ItemSpawningComponent spawningComponent = spawner.GetSpawningComponent();
					
					if (spawningComponent && spawningComponent.HasConfig() && spawningComponent.HaveItemsProcessed())
					{
						items = spawningComponent.GetItems();
					}
					else
					{
						items = m_aItems;
					}
					
					CE_Item item = SelectItem(items, spawner);
						
					if (item && TryToSpawnItem(spawner, item))
					{
						processedSpawnersArray.RemoveItem(spawner);
						runCount++;
					}
					else
					{
						failCount++;
						continue;
					}
				}
				else	
				{
					failCount++;
					continue;
				}
			}
		}
	}
	
	float m_fTestTimer = 0;
	float m_fTestFrequency = 5;
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, handles timing for spawning
	override event protected void OnUpdate(ESystemPoint point)
	{	
		super.OnUpdate(point);
		
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
		if (point != ESystemPoint.FixedFrame)
			return;
		
		float testTimeSlice = GetWorld().GetFixedTimeSlice();
		m_fTestTimer += testTimeSlice;
		
		if (m_fTestTimer >= m_fTestFrequency)
		{
			m_fTestTimer = 0;
			
			PrintFormat("Usage Areas Queried: %1 - Total Count: %2", GetUsageAreasQueried(), m_aUsageAreas.Count());
			PrintFormat("Tier Areas Queried: %1 - Total Count: %2", GetTierAreasQueried(), m_aTierAreas.Count());
			
			Print("Item Count: " + GetItemCount());
		}
		
		if (GetUsageAreasQueried() >= m_aUsageAreas.Count() 
		&& GetTierAreasQueried() >= m_aTierAreas.Count()
		&& !m_bInitiallyRan)
		{
			PrintFormat("Usage Areas Queried: %1 - Total Count: %2", GetUsageAreasQueried(), m_aUsageAreas.Count());
			PrintFormat("Tier Areas Queried: %1 - Total Count: %2", GetTierAreasQueried(), m_aTierAreas.Count());
			
			SetHaveAreasQueried(true);
			
			InitialSpawningPhase();
			
			ProcessContainers();
		}
		
		
		
		if (GetItemCount() >= m_iSpawnerRatioCount)
			return;
		
		if (m_aItems && !m_aItems.IsEmpty() && m_bInitiallyRan)
		{
			float timeSlice = GetWorld().GetFixedTimeSlice();
			m_fTimer += timeSlice;
			
			if (m_fItemSpawningFrequency > 0 && m_fTimer >= m_fItemSpawningFrequency)
			{
				m_fTimer = 0;
				
				CE_Spawner spawner = SelectSpawner(ProcessSpawners());
				
				if (spawner)
				{
					array<ref CE_Item> items = {};
					
					CE_ItemSpawningComponent spawningComponent = spawner.GetSpawningComponent();
					
					if (spawningComponent && spawningComponent.HasConfig() && spawningComponent.HaveItemsProcessed())
					{
						items = spawningComponent.GetItems();
					}
					else
					{
						items = m_aItems;
					}
					
					CE_Item item = SelectItem(items, spawner);
					
					if (item)
						TryToSpawnItem(spawner, item);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes the config and processes each item data into a CE_Item, as long as they pass certains checks
	array<ref CE_Item> ProcessItemData(CE_ItemDataConfig config)
	{	
		array<ref CE_Item> itemsProcessed = {};
		array<ref CE_ItemData> itemDataToBeProcessed = {};
		
		if (config && config.m_ItemData)
		{	
			foreach (CE_ItemData itemData : config.m_ItemData)
			{
				itemDataToBeProcessed.Insert(itemData)
			}
			
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
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.m_sName + " is missing information! Please fix!", LogLevel.ERROR);
					itemDataToBeProcessed.Remove(0);
					itemCount--;
					continue;
				}
				else if (itemData.m_iNominal < itemData.m_iMinimum)
				{
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.m_sName + " has a nominal value less than the minimum value! Please fix!", LogLevel.ERROR);
					itemDataToBeProcessed.Remove(0);
					itemCount--;
					continue;
				}
				else
				{
					CE_Item item = new CE_Item();
					
					item.SetItemData(itemData);
					item.SetTiers(itemData.m_ItemTiers);
					item.SetUsages(itemData.m_ItemUsages);
					item.SetCategory(itemData.m_ItemCategory);
					item.SetAvailableCount(m_RandomGen.RandIntInclusive(itemData.m_iMinimum, itemData.m_iNominal));
					
					if (!itemsProcessed.Contains(item))
					{
						itemsProcessed.Insert(item);
					}
				}
				
				itemDataToBeProcessed.Remove(0);
				itemCount--;
			}
			
			return itemsProcessed;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes each CE_ItemSpawningComponent and processes them into a CE_Spawner, as long as they pass certain checks
	protected array<ref CE_Spawner> ProcessSpawners()
	{
		array<ref CE_Spawner> spawnersProcessed = {};
		array<CE_ItemSpawningComponent> spawnersToBeProcessed = {};
		
		spawnersToBeProcessed.Copy(m_aSpawnerComponents);
		
		int spawnerCount = spawnersToBeProcessed.Count();
		
		for (int i = 0, maxCount = Math.Min(spawnerCount, m_aSpawnerComponents.Count()); i < maxCount; i++)
		{
			CE_ItemSpawningComponent spawnerComponent = spawnersToBeProcessed[0];
			if (!spawnerComponent)
			{
				spawnersToBeProcessed.Remove(0);
				spawnerCount--;
				continue;
			}
			
			if (!spawnerComponent.GetSpawnerUsage() 
			|| !spawnerComponent.GetSpawnerTier() 
			|| !spawnerComponent.GetSpawnerCategories()
			|| spawnerComponent.m_iSpawnerResetTime == 0)
			{
				//Print("[CentralEconomy::CE_ItemSpawningSystem] " + spawnerComponent + " is missing information! Please fix!", LogLevel.ERROR);
				spawnersToBeProcessed.Remove(0);
				spawnerCount--;
				
				if (m_aSpawnerComponents.Contains(spawnerComponent))
				{
					m_aSpawnerComponents.RemoveItem(spawnerComponent); // if missing info, just remove permanently
				}
				
				continue;
			}
			else
			{
				CE_Spawner spawner = new CE_Spawner(spawnerComponent, spawnerComponent.GetItemSpawned(), spawnerComponent.IsReadyForItem());
				
				if (!spawnersProcessed.Contains(spawner))
				{
					spawnersProcessed.Insert(spawner);
				}
			}
			
			spawnersToBeProcessed.Remove(0);
			spawnerCount--;
		}
		
		return spawnersProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes a CE_SearchableContainerComponent and processes it into a CE_SearchableContainer, as long as it passes certain checks
	protected void ProcessContainers()
	{
		array<ref CE_SearchableContainer> containersProcessed = {};
		array<CE_SearchableContainerComponent> containersToBeProcessed = {};
		
		containersToBeProcessed.Copy(m_aContainerComponents);
		
		int containerCount = containersToBeProcessed.Count();
		
		for (int i = 0, maxCount = Math.Min(containerCount, m_aContainerComponents.Count()); i < maxCount; i++)
		{
			CE_SearchableContainerComponent containerComponent = containersToBeProcessed[0];
			if (!containerComponent)
			{
				containersToBeProcessed.Remove(0);
				containerCount--;
				continue;
			}
		
			if (!containerComponent.GetContainerUsage() 
			|| !containerComponent.GetContainerTier() 
			|| !containerComponent.GetContainerCategories()
			|| containerComponent.m_iContainerResetTime == 0)
			{
				containersToBeProcessed.Remove(0);
				containerCount--;
				
				if (m_aContainerComponents.Contains(containerComponent))
				{
					m_aContainerComponents.RemoveItem(containerComponent); // if missing info, just remove permanently
				}
				
				continue;
			}
			else if (containerComponent.GetContainerItemMinimum() > containerComponent.GetContainerItemMaximum())
			{
				Print("[CentralEconomy::CE_ItemSpawningSystem] " + containerComponent + " has a item maximum value less than the item minimum value! Please fix!", LogLevel.ERROR);
				
				containersToBeProcessed.Remove(0);
				containerCount--;
				
				if (m_aContainerComponents.Contains(containerComponent))
				{
					m_aContainerComponents.RemoveItem(containerComponent); // if wrong info, just remove permanently
				}
				
				continue;
			}
			else
			{
				const RplId containerRplId = Replication.FindItemId(containerComponent);
				
				CE_SearchableContainer container = new CE_SearchableContainer();
				
				if (containerRplId)
					container.SetContainerRplId(containerRplId);
				
				foreach (CE_Item item : containerComponent.GetItemsSpawned())
				{
					container.GetItemsSpawned().Insert(item);
				}
				
				container.SetReadyForItems(containerComponent.IsReadyForItems());
				
				containerComponent.SetContainer(container);
				
				Replication.BumpMe();
			}
			
			containersToBeProcessed.Remove(0);
			containerCount--;
		}
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Takes each CE_ItemSpawningComponent and processes them into a CE_Spawner, as long as they pass certain checks
	CE_SearchableContainer ProcessContainer(CE_SearchableContainerComponent containerComponent)
	{
		if (!containerComponent.GetContainerUsage() 
		|| !containerComponent.GetContainerTier() 
		|| !containerComponent.GetContainerCategories()
		|| containerComponent.m_iContainerResetTime == 0)
		{
			if (m_aContainerComponents.Contains(containerComponent))
			{
				m_aContainerComponents.RemoveItem(containerComponent); // if missing info, just remove permanently
			}
		}
		else if (containerComponent.GetContainerItemMinimum() > containerComponent.GetContainerItemMaximum())
		{
			Print("[CentralEconomy::CE_ItemSpawningSystem] " + containerComponent + " has a item maximum value less than the item minimum value! Please fix!", LogLevel.ERROR);
			if (m_aContainerComponents.Contains(containerComponent))
			{
				m_aContainerComponents.RemoveItem(containerComponent); // if missing info, just remove permanently
			}
		}
		else
		{
			CE_SearchableContainer spawner = new CE_SearchableContainer(containerComponent, containerComponent.GetItemsSpawned(), containerComponent.IsReadyForItems());
			
			return spawner;
		}
		
		return null;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Randomly selects a CE_Spawner from a CE_Spawner array, and checks if it's ready for an item
	protected CE_Spawner SelectSpawner(array<ref CE_Spawner> spawnersArray)
	{
		if (spawnersArray.IsEmpty())
			return null;
		
		array<ref CE_Spawner> spawnerArrayCopy = {};
		
		foreach (CE_Spawner spawner : spawnersArray)
		{
			spawnerArrayCopy.Insert(spawner);
		}
		
		for (int i = 0; i < spawnerArrayCopy.Count(); i++)
		{
			int randomIndex = m_RandomGen.RandInt(0, spawnerArrayCopy.Count());
			
			CE_Spawner spawnerSelected = spawnerArrayCopy[randomIndex];
			
			if (spawnerSelected.IsReadyForItem()
			&& !spawnerSelected.GetItemSpawned())
			{
				return spawnerSelected;
			}
			else
			{
				spawnerArrayCopy.RemoveItem(spawnerSelected);
				continue;
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Randomly selects a CE_Item from a CE_Item array, and checks for matching attributes against a CE_Spawner
	protected CE_Item SelectItem(array<ref CE_Item> itemsArray, CE_Spawner spawner)
	{
		if (itemsArray.IsEmpty())
			return null;
		
		array<ref CE_Item> itemsArrayCopy = {};
		
		foreach (CE_Item item : itemsArray)
		{
			itemsArrayCopy.Insert(item);
		}
		
		if (itemsArrayCopy.IsEmpty())
			return null;
		
		CE_ItemSpawningComponent spawnerComponent = spawner.GetSpawningComponent();
		if (!spawnerComponent)
			return null;
		
		for (int i = 0; i < itemsArrayCopy.Count(); i++)
		{
			int randomIndex = m_RandomGen.RandInt(0, itemsArrayCopy.Count());
			if (!randomIndex)
				return null;
			
			CE_Item itemSelected = itemsArrayCopy[randomIndex];
			if (!itemSelected)
				return null;
			
			int itemCount = itemSelected.GetAvailableCount();
			if (!itemCount)
				return null;
			
			if (itemSelected.GetTiers() & spawnerComponent.GetSpawnerTier() 
			&& itemSelected.GetUsages() & spawnerComponent.GetSpawnerUsage() 
			&& itemSelected.GetCategory() & spawnerComponent.GetSpawnerCategories()
			&& itemCount > 0)
			{
				return itemSelected;
			}
			else
			{
				itemsArrayCopy.RemoveItem(itemSelected);
				continue;
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Randomly selects a CE_Item from a CE_Item array, and checks for matching attributes against a CE_SearchableContainer
	array<ref CE_Item> SelectItems(array<ref CE_Item> itemsArray, CE_SearchableContainer container, int min, int max)
	{
		if (itemsArray.IsEmpty())
			return null;
		
		if (!container)
			return null;
		
		CE_SearchableContainerComponent containerComponent = container.GetContainerComponentFromRplId(container.GetContainerRplId());
		if (!containerComponent)
			return null;
		
		int itemsCount = m_RandomGen.RandIntInclusive(min, max);
		if (itemsCount == 0)
			return null;
		
		array<ref CE_Item> itemsArrayCopy = {};
		array<ref CE_Item> itemsSelected = {};
		
		foreach (CE_Item item : itemsArray)
		{
			itemsArrayCopy.Insert(item);
		}
		
		if (itemsArrayCopy.IsEmpty())
			return null;
		
		while (itemsSelected.Count() < itemsCount && itemsArrayCopy.Count() != 0)
		{
			int randomIndex = m_RandomGen.RandInt(0, itemsArrayCopy.Count());
			
			CE_Item itemSelected = itemsArrayCopy[randomIndex];
			if (!itemSelected)
				continue;
			
			if (itemSelected.GetAvailableCount() > 0
			&& itemSelected.GetTiers() & containerComponent.GetContainerTier() 
			&& itemSelected.GetUsages() & containerComponent.GetContainerUsage() 
			&& itemSelected.GetCategory() & containerComponent.GetContainerCategories())
			{
				itemsSelected.Insert(itemSelected);
			}
			else
			{
				itemsArrayCopy.RemoveItem(itemSelected);
				continue;
			}
		}
		
		if (	itemsSelected.IsEmpty())
			return null;
		else
			return itemsSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the CE_Item to the CE_Spawner, returns true if item spawned properly
	protected bool TryToSpawnItem(CE_Spawner spawner, CE_Item item)
	{
		/*
		if (!Replication.IsServer())
			return false;
		*/
		
		if (item && spawner)
		{
			CE_ItemSpawningComponent spawnerComponent = spawner.GetSpawningComponent();
			if (!spawnerComponent)
				return false;
			
			CE_ItemData itemData = item.GetItemData();
			if (!itemData)
				return false;
			
			IEntity spawnEntity = spawnerComponent.GetOwner();
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
			
			item.SetAvailableCount(item.GetAvailableCount() - 1);
			
			// if quantities are set greater than -1 within the item data config
			if (itemData.m_iQuantityMinimum > -1 
			&& itemData.m_iQuantityMaximum > -1)
			{
				// making sure quantity maximum is not less than quantity minimum
				if (itemData.m_iQuantityMinimum <= itemData.m_iQuantityMaximum)
				{
					SetQuantities(newEnt, itemData.m_iQuantityMinimum, itemData.m_iQuantityMaximum);
				}
				else // else you get this error in config
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.m_sName + " has quantity minimum set to more than quantity maximum! Please fix!", LogLevel.ERROR);
			}
			
			// if item is ACTUALLY a vehicle
			Vehicle vehicle = Vehicle.Cast(newEnt);
			if (vehicle)
			{
				// vehicle enter/leave event
		        EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(vehicle.FindComponent(EventHandlerManagerComponent));
		        if (eventHandler)
		        {
		           	eventHandler.RegisterScriptHandler("OnCompartmentEntered", this, spawnerComponent.OnVehicleActivated, true, false);
					eventHandler.RegisterScriptHandler("OnCompartmentLeft", this, spawnerComponent.OnVehicleDeactivated, true, false);
		        }
			}
			
			spawnerComponent.GetItemSpawnedInvoker().Invoke(newEnt, item);
			
			SetItemCount(GetItemCount() + 1);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the CE_Item to the CE_SearchableContainer
	void TryToSpawnItems(CE_SearchableContainer container, array<ref CE_Item> itemsArray)
	{
		CE_SearchableContainerComponent containerComp = container.GetContainerComponentFromRplId(container.GetContainerRplId());
		if (!containerComp)
			return;
		
		IEntity containerEntity = containerComp.GetOwner();
		if (!containerEntity)
			return;
		
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(containerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return;
		
		SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(containerEntity.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!ownerStorage)
			return;
		
		foreach (CE_Item item: itemsArray)
		{
			CE_ItemData itemData = item.GetItemData();
			if (!itemData)
				continue;
			
			storageManager.TrySpawnPrefabToStorage(itemData.m_sPrefab, ownerStorage);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the quantities for the item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values)
	protected void SetQuantities(IEntity ent, int quantMin, int quantMax)
	{
		float randomFloat = Math.RandomIntInclusive(quantMin, quantMax) / 100; // convert it to a decimal to be multiplied later on
		if (!randomFloat)
			return;
		
		float quantity = Math.Round(randomFloat * 10) / 10;
		if (!quantity)
			return;
		
		ResourceName prefabName = ent.GetPrefabData().GetPrefabName();
		
		Vehicle vehicle = Vehicle.Cast(ent);
		
		if (prefabName && prefabName.Contains("Magazine_") || prefabName && prefabName.Contains("Box_"))
		{
			MagazineComponent magComp = MagazineComponent.Cast(ent.FindComponent(MagazineComponent));
			if (!magComp)
				return;
			
			magComp.SetAmmoCount(magComp.GetMaxAmmoCount() * quantity);
		}
		else if (prefabName && prefabName.Contains("Jerrycan_") || vehicle)
		{
			FuelManagerComponent fuelManager = FuelManagerComponent.Cast(ent.FindComponent(FuelManagerComponent));
			if (!fuelManager)
				return;
			
			array<BaseFuelNode> outNodes = {};
			
			fuelManager.GetFuelNodesList(outNodes);
			
			foreach (BaseFuelNode fuelNode : outNodes)
			{
				fuelNode.SetFuel(fuelNode.GetMaxFuel() * quantity);
			}
		}
	}
	
	// GameSystem stuff
	
	//------------------------------------------------------------------------------------------------
	//! Registers spawning component
	void RegisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aSpawnerComponents.Contains(component))
		{
			m_aSpawnerComponents.Insert(component);
			
			if (component.HasConfig() && !component.HaveItemsProcessed())
			{
				array<ref CE_Item> itemsArray = component.GetItems();
				
				foreach (CE_Item item : ProcessItemData(component.GetConfig()))
				{
					itemsArray.Insert(item);
				}
				
				component.SetHaveItemsProcessed(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawning component
	void UnregisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		m_aSpawnerComponents.RemoveItem(component);
		
		if (m_aSpawnerComponents.IsEmpty() && m_aContainerComponents.IsEmpty())
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
		
		if (m_aSpawnerComponents.IsEmpty() && m_aContainerComponents.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers UsageTriggerArea
	void RegisterUsageArea(notnull CE_UsageTriggerArea area)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aUsageAreas.Contains(area))
		{
			m_aUsageAreas.Insert(area);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregisters UsageTriggerArea
	void UnregisterUsageArea(notnull CE_UsageTriggerArea area)
	{
		m_aUsageAreas.RemoveItem(area);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers TierTriggerArea
	void RegisterTierArea(notnull CE_TierTriggerArea area)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aTierAreas.Contains(area))
		{
			m_aTierAreas.Insert(area);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregisters TierTriggerArea
	void UnregisterTierArea(notnull CE_TierTriggerArea area)
	{
		m_aTierAreas.RemoveItem(area);
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Gets m_aItems array
	array<ref CE_Item> GetItems()
	{
		return m_aItems;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Gets m_iItemCount
	int GetItemCount()
	{
		return m_iItemCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets m_iItemCount
	void SetItemCount(int count)
	{
		m_iItemCount = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the m_iUsageAreasQueried count
	int GetUsageAreasQueried()
	{
		return m_iUsageAreasQueried;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the m_iUsageAreasQueried count
	void SetUsageAreasQueried(int count)
	{
		m_iUsageAreasQueried = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the m_iTierAreasQueried count
	int GetTierAreasQueried()
	{
		return m_iTierAreasQueried;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the m_iTierAreasQueried count
	void SetTierAreasQueried(int count)
	{
		m_iTierAreasQueried = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Have usage and tier areas finished querying?
	bool HaveAreasQueried()
	{
		return m_bHaveAreasQueried;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if usage and tier areas finished querying
	void SetHaveAreasQueried(bool queried)
	{
		m_bHaveAreasQueried = queried;
	}
}

void CE_ItemSpawned(IEntity itemEntity, CE_Item item);
typedef func CE_ItemSpawned;
typedef ScriptInvokerBase<CE_ItemSpawned> CE_OnItemSpawnedInvoker;

void CE_ItemDespawned(IEntity itemEntity, CE_Item item);
typedef func CE_ItemDespawned;
typedef ScriptInvokerBase<CE_ItemDespawned> CE_OnItemDespawnedInvoker;

void CE_SpawnerReset(CE_ItemSpawningComponent spawner);
typedef func CE_SpawnerReset;
typedef ScriptInvokerBase<CE_SpawnerReset> CE_OnSpawnerResetInvoker;

void CE_ContainerSearched(CE_SearchableContainer container, IEntity userEntity);
typedef func CE_ContainerSearched;
typedef ScriptInvokerBase<CE_ContainerSearched> CE_OnContainerSearchedInvoker;

void CE_ContainerReset(CE_SearchableContainerComponent container);
typedef func CE_ContainerReset;
typedef ScriptInvokerBase<CE_ContainerReset> CE_OnContainerResetInvoker;
