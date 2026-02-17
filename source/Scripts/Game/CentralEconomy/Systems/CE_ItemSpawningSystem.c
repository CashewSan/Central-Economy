class CE_ItemSpawningSystem : GameSystem
{
	/* 
		To keep this brief, this is the brains of CentralEconomy. Handles all item & spawner processing, and then spawning items to spawners
	
		Enjoy!
			- Cashew
	*/
	
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents		 	= new array<CE_ItemSpawningComponent>(); 			// ALL registered spawner components in the world
	protected ref array<CE_SearchableContainerComponent> 	m_aContainerComponents 			= new array<CE_SearchableContainerComponent>(); 	// ALL registered searchable container components in the world
	//protected ref array<CE_ItemSpawnableComponent> 			m_aSpawnableComponents 			= new array<CE_ItemSpawnableComponent>; 			// ALL CE_ItemSpawnableComponents registered to this system
	protected ref array<ref CE_Item>						m_aItems 						= new array<ref CE_Item>();							// processed CE_ItemData into CE_Item
	protected ref array<ref CE_Spawner>						m_aSpawners 					= new array<ref CE_Spawner>();						// processed spawning entities into CE_Spawners
	protected ref array<CE_UsageTriggerArea>				m_aUsageAreas					= new array<CE_UsageTriggerArea>();					// usage areas registered to the system
	protected ref array<CE_TierTriggerArea>					m_aTierAreas					= new array<CE_TierTriggerArea>();					// tier areas registered to the system
	
	protected ref RandomGenerator 							m_RandomGen						= new RandomGenerator();							// vanilla random generator
	protected ref CE_OnAreasQueriedInvoker					m_OnAreasQueriedInvoker			= new CE_OnAreasQueriedInvoker();					// script invoker for when all areas have been queried
	protected CE_WorldValidationComponent 					m_WorldValidationComponent;															// world validation component in the gamemode
	protected ref CE_ItemDataConfig 						m_Config;																			// config found in profile folder
	
	protected float										m_fInitialDelayTimer				= 0;												// timer for the initial delay before doing the initial spawning of items
	protected float										m_fInitialDelayTime					= 1;													// amount of time for the initial delay
	protected float 									m_fTimer							= 0;												// timer for spawning check interval
	protected float										m_fItemSpawningFrequency			= 0; 												// frequency (in seconds) that an item will spawn
	protected float										m_fItemSpawningRatio				= 0;												// ratio of items the system will aim to spawn compared to spawners
	
	protected int										m_iItemCount						= 0;												// count of items currently on a spawner
	protected int										m_iSpawnerRatioCount				= 0;												// count of spawners multiplied by the m_fItemSpawningRatio
	protected int										m_iUsageAreasQueried				= 0;												// count of usage areas queried
	protected int										m_iTierAreasQueried					= 0;												// count of tier areas queried
	
	[RplProp()]
	protected bool										m_bHaveAreasQueried					= false;											// have the usage and tier areas finished querying?
	protected bool										m_bInitialSpawningRan				= false;											// has the system ran it's initial spawning phase?
	protected bool										m_bProcessingSpawn					= false;											// is the system currently processing a spawn?
	protected bool										m_bStoredPersistenceData			= false;											// is there stored persistence data?
	protected bool										m_bPersistenceFinished				= false;											// if there is stored persistence data, is persistence finished loading the proper data for the item spawning system?
	
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Both)
			.AddPoint(ESystemPoint.FixedFrame);
	}
	
	//------------------------------------------------------------------------------------------------
	//! On system start
	override void OnStarted()
	{
		super.OnStarted();
		
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode && systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
		if (GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if (m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed())
			{
				m_fInitialDelayTime = m_WorldValidationComponent.GetInitialDelayTime();
				
				m_OnAreasQueriedInvoker.Insert(PopulateSpawnerArray);
				
				m_fItemSpawningFrequency = m_WorldValidationComponent.GetItemSpawningFrequency();
				m_fItemSpawningRatio = m_WorldValidationComponent.GetItemSpawningRatio();
				
				m_Config = m_WorldValidationComponent.GetItemDataConfig();
				
				/*
				if (!m_aItems.IsEmpty())
					m_aItems.Clear();
				*/
				
				array<ref CE_Item> processedItems = ProcessItemData(m_Config);
				foreach (CE_Item processedItem : processedItems)
				{
					m_aItems.Insert(processedItem);
				}
				
				RemoveObsoleteItemData(m_Config);
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
	//! Populates the m_aSpawners array with processed spawners and sets the m_iSpawnerRatioCount variable for further use
	protected void PopulateSpawnerArray()
	{
		// if there is existing persistence data available, only continue if data is finished deserializing
		if (ExistingPersistenceDataAvailability() && !IsPersistenceFinished())
		{
			// try it again in 1 second, but shouldn't be an issue (hopefully)
			GetGame().GetCallqueue().CallLater(PopulateSpawnerArray, 1000, false);
			return;
		}
		
		Print("MEOW PopulatingSpawnerArray");
		
		array<ref CE_Spawner> processedSpawners = ProcessSpawners();
		foreach (CE_Spawner processedSpawner : processedSpawners)
		{
			m_aSpawners.Insert(processedSpawner);
		}
		
		int spawnersCount = m_aSpawners.Count();
		
		if (spawnersCount == 0)
		{
			Print("[CentralEconomy::CE_ItemSpawningSystem] NO SPAWNERS FOUND IN WORLD!", LogLevel.ERROR);
		}
		else if (spawnersCount == 1)
		{
			m_iSpawnerRatioCount = 1;
		}
		else
			m_iSpawnerRatioCount = Math.Round(spawnersCount * m_fItemSpawningRatio);
		
		InitialSpawningPhase();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles the initial spawning phase, is called when m_aSpawners has been populated
	protected void InitialSpawningPhase()
	{
		// if the initial spawn has already been ran
		if (m_bInitialSpawningRan)
			return;
		
		Print("MEOW InitialSpawningPhase");
		
		array<ref CE_Spawner> processedSpawnersArray = {};
			
		foreach (CE_Spawner spawner : m_aSpawners)
		{
			processedSpawnersArray.Insert(spawner);
		}
		
		if (m_aItems && !m_aItems.IsEmpty())
		{
			int runCount = 0;
			int failCount = 0;
			
			while (runCount < m_iSpawnerRatioCount && failCount < processedSpawnersArray.Count())
			{
				CE_Spawner spawner = SelectSpawner(processedSpawnersArray);
				
				if (spawner)
				{
					array<ref CE_Item> items = {};
					
					//CE_ItemSpawningComponent spawningComponent = spawner.GetSpawningComponent();
					
					IEntity spawnerEntity = spawner.GetSpawnerEntity();
					if (!spawnerEntity)
						continue;
					
					CE_ItemSpawningComponent spawningComponent = CE_ItemSpawningComponent.Cast(spawnerEntity.FindComponent(CE_ItemSpawningComponent));
					if (spawningComponent && spawningComponent.HasConfig() && spawningComponent.HaveItemsProcessed())
					{
						items = spawningComponent.GetItems();
					}
					else
					{
						items = m_aItems;
					}
					
					CE_Item item;
					
					if (!items.IsEmpty())
						item = SelectItem(items, spawner);
						
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
			
			m_bInitialSpawningRan = true;
		}
	}
	
	//float m_fTestTimer = 0;
	//float m_fTestFrequency = 5;
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, handles timing for spawning
	override event protected void OnUpdate(ESystemPoint point)
	{	
		super.OnUpdate(point);
		
		if (point != ESystemPoint.FixedFrame)
			return;
		
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode && systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
		/*
		float testTimeSlice = GetWorld().GetFixedTimeSlice();
		m_fTestTimer += testTimeSlice;
		
		if (m_fTestTimer >= m_fTestFrequency)
		{
			m_fTestTimer = 0;
			
			PrintFormat("Usage Areas Queried: %1 - Total Count: %2", GetUsageAreasQueried(), m_aUsageAreas.Count());
			PrintFormat("Tier Areas Queried: %1 - Total Count: %2", GetTierAreasQueried(), m_aTierAreas.Count());
			
			Print("Item Count: " + GetItemCount());
		}
		*/
		
		if (m_WorldValidationComponent && !m_WorldValidationComponent.HasWorldProcessed())
			return;
		
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		if (m_fInitialDelayTimer < m_fInitialDelayTime)
		{
			m_fInitialDelayTimer += timeSlice;
			return;
		}
		
		if (!HaveAreasQueried() && GetUsageAreasQueried() >= m_aUsageAreas.Count() 
		&& GetTierAreasQueried() >= m_aTierAreas.Count())
		{
			m_bHaveAreasQueried = true;
			Replication.BumpMe();
			
			m_OnAreasQueriedInvoker.Invoke();
			
			m_aUsageAreas.Clear();	// no point of keeping in memory once queried since querying only happens once at launch of server
			m_aTierAreas.Clear();	// no point of keeping in memory once queried since querying only happens once at launch of server
		}
		
		if (GetItemCount() >= m_iSpawnerRatioCount)
			return;
		
		if (m_aItems && !m_aItems.IsEmpty() && m_bInitialSpawningRan)
		{
			m_fTimer += timeSlice;
			
			if (m_fItemSpawningFrequency > 0 && m_fTimer >= m_fItemSpawningFrequency && !m_bProcessingSpawn)
			{
				m_fTimer = 0;
				m_bProcessingSpawn = true;
				
				CE_Spawner spawner = SelectSpawner(m_aSpawners);
				
				Print("MEOW Spawner " + spawner);
				
				if (spawner)
				{
					array<ref CE_Item> items;
					
					//CE_ItemSpawningComponent spawningComponent = spawner.GetSpawningComponent();
					IEntity spawnerEntity = spawner.GetSpawnerEntity();
					if (!spawnerEntity)
						return;
					
					CE_ItemSpawningComponent spawningComponent = CE_ItemSpawningComponent.Cast(spawnerEntity.FindComponent(CE_ItemSpawningComponent));
					if (spawningComponent && spawningComponent.HasConfig() && spawningComponent.HaveItemsProcessed())
					{
						items = spawningComponent.GetItems();
					}
					else
					{
						items = m_aItems;
					}
					
					Print("MEOW Items Count " + items.Count());
					
					if (!items.IsEmpty())
						GetGame().GetCallqueue().CallLater(DelayedItemSelection, (m_fItemSpawningFrequency * 1000) * 0.25, false, spawner, items); // so if spawning frequency is set to 30 seconds, we multiply it by 1000 (to get 30000ms), then multiply by 0.25 to get a total 7500ms (7.5 seconds)
					else
						m_bProcessingSpawn = false;
				}
				else
					m_bProcessingSpawn = false;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Item selection that gets delayed to help split performance usage
	protected void DelayedItemSelection(CE_Spawner spawner, array<ref CE_Item> items)
	{
		CE_Item item = SelectItem(items, spawner);
					
		if (item)
			GetGame().GetCallqueue().CallLater(TryToSpawnItem, (m_fItemSpawningFrequency * 1000) * 0.25, false, spawner, item); // so if spawning frequency is set to 30 seconds, we multiply it by 1000 (to get 30000ms), then multiply by 0.25 to get a total 7500ms (7.5 seconds)
		else
			m_bProcessingSpawn = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes the config and processes each item data into a CE_Item, as long as they pass certains checks
	array<ref CE_Item> ProcessItemData(CE_ItemDataConfig config)
	{
		array<ref CE_Item> itemsProcessed = {};
		array<ref CE_ItemData> itemDataToBeProcessed = {};
		
		if (config && config.GetItemDataArray())
		{	
			foreach (CE_ItemData itemData : config.GetItemDataArray())
			{
				itemDataToBeProcessed.Insert(itemData)
			}
			
			int itemCount = itemDataToBeProcessed.Count();
			
			for (int i = 0, maxCount = Math.Min(itemCount, config.GetItemDataArray().Count()); i < maxCount; i++)
			{
				CE_ItemData itemData = itemDataToBeProcessed[0];
				
				if (!itemData.GetName() 
				|| !itemData.GetPrefab() 
				|| !itemData.GetNominal()
				|| !itemData.GetMinimum() 
				|| !itemData.GetLifetime() 
				|| !itemData.GetRestock() 
				|| !itemData.GetCategory() 
				|| !itemData.GetUsages() 
				|| !itemData.GetTiers())
				{
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.GetName() + " is missing information! Please fix!", LogLevel.ERROR);
					itemDataToBeProcessed.Remove(0);
					itemCount--;
					continue;
				}
				else if (itemData.GetNominal() < itemData.GetMinimum())
				{
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.GetName() + " has a nominal value less than the minimum value! Please fix!", LogLevel.ERROR);
					itemDataToBeProcessed.Remove(0);
					itemCount--;
					continue;
				}
				else if (ItemExistenceCheck(itemData.GetName()))
				{
					//Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.GetName() + " already exists! Will not add to m_aItems!", LogLevel.ERROR);
					itemDataToBeProcessed.Remove(0);
					itemCount--;
					continue;
				}
				else
				{
					CE_Item item = new CE_Item();
					
					item.SetItemUUID(UUID.GenV4());
					item.SetItemDataName(itemData.GetName());
					item.SetTiers(itemData.GetTiers());
					item.SetUsages(itemData.GetUsages());
					item.SetCategory(itemData.GetCategory());
					item.SetAvailableCount(m_RandomGen.RandIntInclusive(itemData.GetMinimum(), itemData.GetNominal()));
					
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
	//! Checks if item already exists within the m_aItems array, returns true if match is found, false otherwise
	protected bool ItemExistenceCheck(string itemName)
	{
		foreach	(CE_Item item : m_aItems)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(item.GetItemDataName()))
				continue;
			
			if (item.GetItemDataName() == itemName)
				return true;
			
			/*
			CE_ItemData itemData = item.GetItemData();
			if (itemData)
			{
				string itemDataName = itemData.GetName();
				if (itemDataName == itemName)
				{
					return true;
				}
			}
			*/
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes the config and removes obsolete items from m_aItems (in the case of if they were removed from the CE_ItemDataConfig, but still exist in persistent m_aItems)
	protected void RemoveObsoleteItemData(CE_ItemDataConfig config)
	{
		if (m_aItems && !m_aItems.IsEmpty() && config && config.GetItemDataArray())
		{
			foreach (CE_Item persistentItem : m_aItems)
			{
				bool safe = false;
				/*
				CE_ItemData persistentItemData = persistentItem.GetItemData();
				if (!persistentItemData)
				{
					Print("[CentralEconomy::CE_ItemSpawningSystem] Something has gone terribly wrong with registering CE_Items to m_aItems! Please fix or report!", LogLevel.ERROR);
					m_aItems.RemoveItem(persistentItem);
					continue;
				}
				
				string persistentItemDataName = persistentItemData.GetName();
				*/
				
				string persistentItemDataName = persistentItem.GetItemDataName();
				
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(persistentItemDataName))
					continue;
				
				foreach (CE_ItemData itemData : config.GetItemDataArray())
				{
					string itemDataName = itemData.GetName();
					
					if (persistentItemDataName == itemDataName)
					{
						safe = true;
						break;
					}
				}
				
				if (!safe)
				{
					m_aItems.RemoveItem(persistentItem);
					Print("[CentralEconomy::CE_ItemSpawningSystem] - Removed obsolete item data: " + persistentItemDataName, LogLevel.WARNING);
				}
			}
		}
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
			|| spawnerComponent.GetSpawnerResetTime() == 0)
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
			else if (!spawnerComponent.GetSpawnerUUID().IsNull())
			{
				Print("[CentralEconomy::CE_ItemSpawningSystem] Potential matching CE_Spawner?");
				
				bool matchFound = false;
				
				foreach (CE_Spawner spawner : m_aSpawners)
				{
					// if matching UUID found, set to corresponding spawner entity to the CE_Spawner
					UUID spawnerUUID = spawner.GetSpawnerUUID();
					if (spawnerUUID == spawnerComponent.GetSpawnerUUID())
					{
						Print("[CentralEconomy::CE_ItemSpawningSystem] Matching CE_Spawner found!");
						matchFound = true;
						spawner.SetSpawnerEntity(spawnerComponent.GetOwner());
						spawnersToBeProcessed.Remove(0);
						spawnerCount--;
						continue;
					}
				}
				
				if (!matchFound)
				{
					Print("[CentralEconomy::CE_ItemSpawningSystem] Matching CE_Spawner not found! Resetting UUID", LogLevel.ERROR);
					spawnerComponent.SetSpawnerUUID(UUID.NULL_UUID); // reset UUID if no match found
				}
				
				continue;
			}
			else
			{
				CE_Spawner spawner = new CE_Spawner();
				
				UUID spawnerUUID = UUID.GenV4();
				
				spawnerComponent.SetSpawnerUUID(spawnerUUID);
				spawner.SetSpawnerUUID(spawnerUUID);
				spawner.SetSpawnerEntity(spawnerComponent.GetOwner());
				spawner.SetSpawnedItemUUID(spawnerComponent.GetSpawnedItemUUID());
				spawner.SetReadyForItem(spawnerComponent.IsReadyForItem());
				
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
	protected void ProcessContainer(notnull CE_SearchableContainerComponent containerComponent)
	{
		if (!containerComponent.GetContainerUsage() 
		|| !containerComponent.GetContainerTier() 
		|| !containerComponent.GetContainerCategories()
		|| containerComponent.GetContainerResetTime() == 0)
		{
			containerComponent.SetReadyForItems(false);
			
			if (m_aContainerComponents.Contains(containerComponent))
			{
				m_aContainerComponents.RemoveItem(containerComponent);
			}
			
			return;
		}
		else if (containerComponent.GetContainerItemMinimum() > containerComponent.GetContainerItemMaximum())
		{
			Print("[CentralEconomy::CE_ItemSpawningSystem] " + containerComponent + " has a item maximum value less than the item minimum value! Please fix!", LogLevel.ERROR);
			
			containerComponent.SetReadyForItems(false);
			
			if (m_aContainerComponents.Contains(containerComponent))
			{
				m_aContainerComponents.RemoveItem(containerComponent);
			}
			
			return;
		}
		else
		{
			foreach (CE_Item item : containerComponent.GetItemsSpawned())
			{
				containerComponent.GetItemsSpawned().Insert(item);
			}
			
			containerComponent.SetReadyForItems(containerComponent.IsReadyForItems());
		}
		
		/*
		if (m_aContainerComponents.Contains(containerComponent))
		{
			m_aContainerComponents.RemoveItem(containerComponent); // remove as they're not really worth to be kept in memory? They only gotta process once, and then rest is handle through useraction
		}
		*/
		
		//Print("Processed Container");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Randomly selects a CE_Spawner from a CE_Spawner array, and checks if it's ready for an item
	protected CE_Spawner SelectSpawner(array<ref CE_Spawner> spawnersArray)
	{
		if (spawnersArray.IsEmpty())
			return null;
		
		Print("MEOW ARRAY NOT EMPTY");
		
		array<ref CE_Spawner> spawnerArrayCopy = {};
		
		foreach (CE_Spawner spawner : spawnersArray)
		{
			spawnerArrayCopy.Insert(spawner);
		}
		
		for (int i = 0; i < spawnerArrayCopy.Count(); i++)
		{
			int randomIndex = m_RandomGen.RandInt(0, spawnerArrayCopy.Count());
			
			CE_Spawner spawnerSelected = spawnerArrayCopy[randomIndex];
			
			Print("MEOW Item Spawned " + spawnerSelected.GetItemSpawned());
			
			if (spawnerSelected.IsReadyForItem()
			&& !spawnerSelected.GetItemSpawned())
			{
				Print("MEOW SPAWNER SELECTED");
				return spawnerSelected;
			}
			else
			{
				Print("MEOW SPAWNER REMOVED");
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
		array<ref CE_Item> itemsArrayCopy = {};
		
		foreach (CE_Item item : itemsArray)
		{
			itemsArrayCopy.Insert(item);
		}
		
		if (itemsArrayCopy.IsEmpty())
			return null;
		
		//CE_ItemSpawningComponent spawnerComponent = spawner.GetSpawningComponent();
		
		IEntity spawnerEntity = spawner.GetSpawnerEntity();
		if (!spawnerEntity)
			return null;
		
		CE_ItemSpawningComponent spawnerComponent = CE_ItemSpawningComponent.Cast(spawnerEntity.FindComponent(CE_ItemSpawningComponent));
		if (!spawnerComponent)
			return null;
		
		for (int i = 0; i < itemsArrayCopy.Count(); i++)
		{
			int randomIndex = m_RandomGen.RandInt(0, itemsArrayCopy.Count());
			
			CE_Item itemSelected = itemsArrayCopy[randomIndex];
			if (!itemSelected)
				return null;
			
			int itemCount = itemSelected.GetAvailableCount();
			
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
	array<ref CE_Item> SelectItems(array<ref CE_Item> itemsArray, CE_SearchableContainerComponent containerComponent, int min, int max)
	{
		if (itemsArray.IsEmpty())
			return null;
		
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
		Print("MEOW TryToSpawnItem");
		if (item && spawner)
		{
			IEntity spawnEntity = spawner.GetSpawnerEntity();
			if (!spawnEntity)
				return false;
			
			CE_ItemSpawningComponent spawnerComponent = CE_ItemSpawningComponent.Cast(spawnEntity.FindComponent(CE_ItemSpawningComponent));
			if (!spawnerComponent)
				return false;
			
			string itemDataName = item.GetItemDataName();
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(itemDataName))
				return false;
			
			CE_ItemData itemData = FindItemDataByName(itemDataName);
			if (!itemData)
				return false;
		
			vector m_WorldTransform[4];
			spawnEntity.GetWorldTransform(m_WorldTransform);
			
			if (itemData.GetRotation() != vector.Zero)
				Math3D.AnglesToMatrix(itemData.GetRotation(), m_WorldTransform);
			
			EntitySpawnParams params();
			m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.025; // to not make item be in the ground
			params.Transform = m_WorldTransform;
			
			Resource m_Resource = Resource.Load(itemData.GetPrefab());
			if (!m_Resource)
				return false;
			
			IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, spawnEntity.GetWorld(), params);
			if (!newEnt)
				return false;
			
			item.SetAvailableCount(item.GetAvailableCount() - 1);
			
			// if quantities are set greater than -1 within the item data config
			if (itemData.GetQuantityMinimum() > -1 
			&& itemData.GetQuantityMaximum() > -1)
			{
				// making sure quantity maximum is not less than quantity minimum
				if (itemData.GetQuantityMinimum() <= itemData.GetQuantityMaximum())
				{
					SetQuantities(newEnt, itemData.GetQuantityMinimum(), itemData.GetQuantityMaximum());
				}
				else // else you get this error in console
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.GetName() + " has quantity minimum set to more than quantity maximum! Please fix!", LogLevel.ERROR);
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
			
			spawnerComponent.GetItemSpawnedInvoker().Invoke(newEnt, item, spawner);
			
			m_iItemCount = GetItemCount() + 1;
			
			m_bProcessingSpawn = false;
			return true;
		}
		
		m_bProcessingSpawn = false;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the CE_Item to the CE_SearchableContainer
	void TryToSpawnItems(CE_SearchableContainerComponent containerComp, array<ref CE_Item> itemsArray)
	{
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode && systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
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
			string itemDataName = item.GetItemDataName();
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(itemDataName))
				return;
			
			CE_ItemData itemData = FindItemDataByName(itemDataName);
			if (!itemData)
				return;
			
			vector worldTransform[4];
			containerEntity.GetWorldTransform(worldTransform);
			
			EntitySpawnParams params();
			params.Transform = worldTransform;
			
			Resource resource = Resource.Load(itemData.GetPrefab());
			if (!resource)
				return;
			
			IEntity newEnt = GetGame().SpawnEntityPrefab(resource, containerEntity.GetWorld(), params);
			if (!newEnt)
				return;
			
			/*
			if (storageManager.TrySpawnPrefabToStorage(itemData.m_sPrefab, ownerStorage))
				item.SetAvailableCount(item.GetAvailableCount() - 1);
			*/
			
			CE_ItemSpawnableComponent spawnableItem = CE_ItemSpawnableComponent.Cast(newEnt.FindComponent(CE_ItemSpawnableComponent));
			if (spawnableItem)
			{
				spawnableItem.SetWasDepositedByAction(true);
				spawnableItem.HookDepositEvent();
			}
			else
			{
				Print("[CentralEconomy::CE_ItemSpawningSystem] THIS ITEM HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + itemData.GetName(), LogLevel.ERROR);
			}
			
			if (!storageManager.CanInsertItemInStorage(newEnt, ownerStorage))
			{
				//Print("entity deleted");
				
				SCR_EntityHelper.DeleteEntityAndChildren(newEnt);
			}
			else
			{	
				storageManager.InsertItem(newEnt, ownerStorage);
				
				if (spawnableItem)
				{
					spawnableItem.GetItemDepositedInvoker().Invoke(item, containerComp);
				}
				
				item.SetAvailableCount(item.GetAvailableCount() - 1);
				
				// if quantities are set greater than -1 within the item data config
				if (itemData.GetQuantityMinimum() > -1 
				&& itemData.GetQuantityMaximum() > -1)
				{
					// making sure quantity maximum is not less than quantity minimum
					if (itemData.GetQuantityMinimum() <= itemData.GetQuantityMaximum())
					{
						SetQuantities(newEnt, itemData.GetQuantityMinimum(), itemData.GetQuantityMaximum());
					}
					else // else you get this error in console
						Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.GetName() + " has quantity minimum set to more than quantity maximum! Please fix!", LogLevel.ERROR);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the quantities for the item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values)
	protected void SetQuantities(IEntity ent, int quantMin, int quantMax)
	{
		float randomFloat = Math.RandomIntInclusive(quantMin, quantMax) / 100; // convert it to a decimal to be multiplied later on
		
		float quantity = Math.Round(randomFloat * 10) / 10;
		
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
	
	//------------------------------------------------------------------------------------------------
	// GAMESYSTEM STUFF
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Returns instance of system within the entity's world
	static CE_ItemSpawningSystem GetByEntityWorld(IEntity entity)
	{
		World world = entity.GetWorld();
		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns instance of system within the entity's world
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
			
			ProcessContainer(component);
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
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Registers spawning component
	void RegisterSpawnable(notnull CE_ItemSpawnableComponent component)
	{
		if (!m_aSpawnableComponents.Contains(component))
		{
			m_aSpawnableComponents.Insert(component);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawning component
	void UnregisterSpawnable(notnull CE_ItemSpawnableComponent component)
	{
		m_aSpawnableComponents.RemoveItem(component);
	}
	*/
	
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
	//! Destructor, clear arrays from memory
	protected void ~CE_ItemSpawningSystem()
	{
		if (!m_aSpawnerComponents.IsEmpty())
			m_aSpawnerComponents.Clear();
		
		if (!m_aContainerComponents.IsEmpty())
			m_aContainerComponents.Clear();
		
		if (!m_aItems.IsEmpty())
			m_aItems.Clear();
		
		if (!m_aSpawners.IsEmpty())
			m_aSpawners.Clear();
		
		if (!m_aUsageAreas.IsEmpty())
			m_aUsageAreas.Clear();
		
		if (!m_aTierAreas.IsEmpty())
			m_aTierAreas.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Finds corresponding CE_Spawner by it's UUID, returns null if not found
	CE_Spawner FindSpawnerByUUID(UUID spawnerUUID)
	{
		if (spawnerUUID.IsNull())
			return null;
		
		foreach (CE_Spawner spawner : m_aSpawners)
		{
			if (spawner.GetSpawnerUUID() && spawner.GetSpawnerUUID() == spawnerUUID)
				return spawner;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds corresponding CE_Item by it's UUID, returns null if not found
	CE_Item FindItemByUUID(UUID itemUUID)
	{
		if (itemUUID.IsNull())
			return null;
		
		foreach (CE_Item item : m_aItems)
		{
			if (item.GetItemUUID() && item.GetItemUUID() == itemUUID)
				return item;
		}
		return null;
	}
		
	/*
	//------------------------------------------------------------------------------------------------
	//! Finds corresponding CE_ItemSpawnableComponent by the item's UUID, returns null if not found
	CE_ItemSpawnableComponent FindItemSpawnableByUUID(UUID itemUUID)
	{
		if (itemUUID.IsNull())
			return null;
		
		foreach (CE_ItemSpawnableComponent spawnableComp : m_aSpawnableComponents)
		{
			if (spawnableComp.GetSpawnedItemUUID() && spawnableComp.GetSpawnedItemUUID() == itemUUID)
				return spawnableComp;
		}
		return null;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Finds corresponding CE_ItemSpawningComponent by the spawner's UUID, returns null if not found
	CE_ItemSpawningComponent FindSpawningComponentByUUID(UUID spawnerUUID)
	{
		if (spawnerUUID.IsNull())
			return null;
		
		foreach (CE_ItemSpawningComponent spawnerComp : m_aSpawnerComponents)
		{
			if (spawnerComp.GetSpawnerUUID() && spawnerComp.GetSpawnerUUID() == spawnerUUID)
				return spawnerComp;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds corresponding CE_ItemData by it's name, returns null if not found
	CE_ItemData FindItemDataByName(string itemDataName)
	{
		if (!m_Config)
			return null;
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(itemDataName))
			return null;
		
		array<ref CE_ItemData> itemDatas = m_Config.GetItemDataArray();
		
		if (!itemDatas || itemDatas && itemDatas.IsEmpty())
			return null;
		
		foreach (CE_ItemData itemData : itemDatas)
		{
			if (itemData.GetName() && itemData.GetName() == itemDataName)
				return itemData;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets m_aSpawnerComponents array
	array<CE_ItemSpawningComponent> GetSpawnerComponents()
	{
		return m_aSpawnerComponents;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets m_aItems array
	array<ref CE_Item> GetItems()
	{
		return m_aItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets m_aSpawners array
	array<ref CE_Spawner> GetSpawners()
	{
		return m_aSpawners;
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
	
	//------------------------------------------------------------------------------------------------
	//! Is there stored persistence data for the item spawning system?
	bool ExistingPersistenceDataAvailability()
	{
		return m_bStoredPersistenceData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if there is persistence data available (true = yes, false = no)
	void SetPersistenceDataAvailability(bool available)
	{
		m_bStoredPersistenceData = available;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is persistence finished loading the proper data for the item spawning system?
	bool IsPersistenceFinished()
	{
		return m_bPersistenceFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the persistence has finished loading (only called from the serializer, don't call otherwise)
	void SetPersistenceFinished(bool finished)
	{
		m_bPersistenceFinished = finished;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has the initial spawning phase already ran?
	bool HasInitialSpawnRan()
	{
		return m_bInitialSpawningRan;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the initial spawning phase has been ran
	void SetHasInitialSpawnRan(bool ran)
	{
		m_bInitialSpawningRan = ran;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets CE_OnAreasQueriedInvoker script invoker
	CE_OnAreasQueriedInvoker GetOnAreasQueriedInvoker()
	{
		return m_OnAreasQueriedInvoker;
	}
}

void CE_AreasQueried();
typedef func CE_AreasQueried;
typedef ScriptInvokerBase<CE_AreasQueried> CE_OnAreasQueriedInvoker;

void CE_ItemSpawnedOnSpawner(notnull IEntity itemEntity, notnull CE_Item item, notnull CE_Spawner spawner);
typedef func CE_ItemSpawnedOnSpawner;
typedef ScriptInvokerBase<CE_ItemSpawnedOnSpawner> CE_OnItemSpawnedOnSpawnerInvoker;

void CE_ItemDespawnedFromSpawner(IEntity itemEntity, CE_Item item);
typedef func CE_ItemDespawnedFromSpawner;
typedef ScriptInvokerBase<CE_ItemDespawnedFromSpawner> CE_OnItemDespawnedFromSpawnerInvoker;

void CE_SpawnerReset(CE_ItemSpawningComponent spawner);
typedef func CE_SpawnerReset;
typedef ScriptInvokerBase<CE_SpawnerReset> CE_OnSpawnerResetInvoker;

void CE_ContainerSearched(CE_SearchableContainerComponent container, IEntity userEntity);
typedef func CE_ContainerSearched;
typedef ScriptInvokerBase<CE_ContainerSearched> CE_OnContainerSearchedInvoker;

void CE_ContainerReset(CE_SearchableContainerComponent container);
typedef func CE_ContainerReset;
typedef ScriptInvokerBase<CE_ContainerReset> CE_OnContainerResetInvoker;
