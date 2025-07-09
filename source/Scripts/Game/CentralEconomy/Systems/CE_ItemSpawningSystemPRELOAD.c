class CE_ItemSpawningSystem : GameSystem
{
	// New variables
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents		 		= new array<CE_ItemSpawningComponent>; 			// ALL registered spawner components in the world
	protected ref array<ref CE_Spawner>					m_aSpawners						= new array<ref CE_Spawner>;						// processed CE_ItemSpawningComponent into CE_Spawner
	protected ref array<CE_SearchableContainerComponent> 	m_aContainers 					= new array<CE_SearchableContainerComponent>; 		// ALL registered searchable container components in the world
	protected ref array<ref CE_Item>						m_aItems 						= new array<ref CE_Item>;							// processed CE_ItemData into CE_Item
	
	protected ref RandomGenerator 						m_RandomGen						= new RandomGenerator();							// vanilla random generator
	
	protected float 										m_fTimer							= 0;												// timer for spawning check interval
	protected float										m_fItemSpawningFrequency			= 0; 											// frequency (in seconds) that an item will spawn
	protected float										m_fItemSpawningRatio				= 0;												// ratio of items the system will aim to spawn compared to spawners
	
	protected int										m_iItemCount						= 0;												// count of items currently on a spawner
	protected int										m_iSpawnerRatioCount				= 0;												// count of spawners multiplied by the m_fItemSpawningRatio
	
	protected bool										m_bInitiallyRan					= false;											// has the system finished it's initial spawning phase?
	
	//------------------------------------------------------------------------------------------------
	//! 
	override void OnInit()
	{
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
				
				//Print("ItemData Count: " + m_Config.m_ItemData.Count());
				
				m_aItems = ProcessItemData(m_Config);
				
				GetGame().GetCallqueue().CallLater(InitialSpawningPhase, 5000);
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
	//!
	protected void InitialSpawningPhase()
	{
		if (!m_aItems.IsEmpty())
		{
			array<ref CE_Spawner> processedSpawnersArray = {};
			
			foreach (CE_Spawner spawner : ProcessSpawners())
			{
				processedSpawnersArray.Insert(spawner);
			}
			
			//Print(processedSpawnersArray.Count());
			
			m_iSpawnerRatioCount = Math.Round(processedSpawnersArray.Count() * m_fItemSpawningRatio);
			int runCount = 0;
			int failCount = 0;
			
			//Print(m_iSpawnerRatioCount);
			
			while (runCount < m_iSpawnerRatioCount && failCount < processedSpawnersArray.Count())
			{
				CE_Spawner spawner = SelectSpawner(processedSpawnersArray);
				
				//Print(spawner);
				
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
						//Print("fail2");
						failCount++;
						continue;
					}
				}
				else	
				{
					//Print("fail1");
					failCount++;
					continue;
				}
			}
			
			//Print(runCount);
			Print(failCount);
			
			m_bInitiallyRan = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	override event protected void OnUpdate(ESystemPoint point)
	{		
		if (GetItemCount() >= m_iSpawnerRatioCount)
			return;
		
		if (!m_aItems.IsEmpty() && m_bInitiallyRan)
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
	//! 
	array<ref CE_Item> ProcessItemData(CE_ItemDataConfig config)
	{	
		array<ref CE_Item> itemsProcessed = {};
		array<ref CE_ItemData> itemDataToBeProcessed = {};
		
		foreach (CE_ItemData itemData : config.m_ItemData)
		{
			itemDataToBeProcessed.Insert(itemData)
		}
		
		//Print("ItemDataToBeProcessed Count: " + itemDataToBeProcessed.Count());
		
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
				Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.m_sName + " is missing information!  Please fix!", LogLevel.ERROR);
				continue;
			}
			else
			{
				CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory, m_RandomGen.RandIntInclusive(itemData.m_iMinimum, itemData.m_iNominal));
					
				if (!itemsProcessed.Contains(item))
				{
					itemsProcessed.Insert(item);
				}
			}
			
			itemDataToBeProcessed.Remove(0);
			itemCount--;
		}
		
		//Print("Item Count: " + itemsProcessed.Count());
		
		return itemsProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected array<ref CE_Spawner> ProcessSpawners()
	{
		array<ref CE_Spawner> spawnersProcessed = {};
		array<CE_ItemSpawningComponent> spawnersToBeProcessed = {};
		
		spawnersToBeProcessed.Copy(m_aSpawnerComponents);
		
		//Print("SpawnersToBeProcessed Count: " + spawnersToBeProcessed.Count());
		
		int spawnerCount = spawnersToBeProcessed.Count();
		
		for (int i = 0, maxCount = Math.Min(spawnerCount, m_aSpawnerComponents.Count()); i < maxCount; i++)
		{
			CE_ItemSpawningComponent spawnerComponent = spawnersToBeProcessed[0];
			if (!spawnerComponent)
				continue;
			
			Print("Spawner Tier: " + spawnerComponent.GetSpawnerTier());
			Print("Spawner Usage: " + spawnerComponent.GetSpawnerUsage());
			
			if (!spawnerComponent.GetSpawnerUsage() 
			|| !spawnerComponent.GetSpawnerTier() 
			|| !spawnerComponent.m_Categories
			|| !spawnerComponent.m_iSpawnerResetTime)
			{
				Print("[CentralEconomy::CE_ItemSpawningSystem] " + spawnerComponent + " is missing information! Please fix!", LogLevel.ERROR);
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
		
		//Print("Spawner Count: " + spawnersProcessed.Count());
		
		return spawnersProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
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
	//! 
	protected CE_Item SelectItem(array<ref CE_Item> itemsArray, CE_Spawner spawner)
	{
		if (itemsArray.IsEmpty())
			return null;
		
		array<ref CE_Item> itemsArrayCopy = {};
		
		int probabilityTotal = 0;
		
		foreach (CE_Item item : itemsArray)
		{
			itemsArrayCopy.Insert(item);
			
			probabilityTotal = probabilityTotal + item.GetAvailableCount();
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
			&& itemSelected.GetCategory() & spawnerComponent.m_Categories
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
	//! Tries to spawn the item, returns true if item spawned properly
	protected bool TryToSpawnItem(CE_Spawner spawner, CE_Item item)
	{
		if (!Replication.IsServer())
			return false;
		
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
			
			if (itemData.m_iQuantityMinimum > -1 
			&& itemData.m_iQuantityMaximum > -1)
			{
				if (itemData.m_iQuantityMinimum <= itemData.m_iQuantityMaximum)
				{
					SetQuantities(newEnt, itemData.m_iQuantityMinimum, itemData.m_iQuantityMaximum);
				}
				else
					Print("[CentralEconomy::CE_ItemSpawningSystem] " + itemData.m_sName + " has quantity minimum set to more than quanity maximum! Please fix!", LogLevel.ERROR);
			}
			
			// if item is a vehicle
			Vehicle vehicle = Vehicle.Cast(newEnt);
			if (vehicle)
			{
				// Vehicle enter/leave event
		        EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(newEnt.FindComponent(EventHandlerManagerComponent));
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
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void SetQuantities(IEntity ent, int quantMin, int quantMax)
	{
		float randomFloat = Math.RandomIntInclusive(quantMin, quantMax) / 100; // convert it to a decimal to multiplied later on
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
	
	//------------------------------------------------------------------------------------------------
	//! Chooses a randomized number between 0 and the probabilityTotal to the select and item it it's value is greater than said randomized number
	protected bool Probability(float probability, float probabilityTotal)
	{
		//Print("Probability: " + probability);
		
		int random = m_RandomGen.RandInt(0, probabilityTotal);
		
		return probability >= random;
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
			
			if (component && component.HasConfig() && !component.HaveItemsProcessed())
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
		
		if (m_aSpawnerComponents.IsEmpty())
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
	//! Gets m_aItems array
	array<ref CE_Item> GetItems()
	{
		return m_aItems;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	int GetItemCount()
	{
		return m_iItemCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetItemCount(int count)
	{
		m_iItemCount = count;
	}
}

class CE_Item
{
	protected CE_ItemData ItemData;
	protected CE_ELootTier Tiers;
	protected CE_ELootUsage Usages;
	protected CE_ELootCategory Category;
	protected int AvailableCount;
	
	//------------------------------------------------------------------------------------------------
	void CE_Item(CE_ItemData itemData, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category, int availableCount)
	{
		ItemData = itemData;
		Tiers = tiers;
		Usages = usages;
		Category = category;
		AvailableCount = availableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_ItemData GetItemData()
	{
		return ItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetItemData(CE_ItemData itemData)
	{
		ItemData = itemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_ELootTier GetTiers()
	{
		return Tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetTiers(CE_ELootTier tiers)
	{
		Tiers = tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_ELootUsage GetUsages()
	{
		return Usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetUsages(CE_ELootUsage usages)
	{
		Usages = usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_ELootCategory GetCategory()
	{
		return Category;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetCategory(CE_ELootCategory category)
	{
		Category = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	int GetAvailableCount()
	{
		return AvailableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetAvailableCount(int availableCount)
	{
		AvailableCount = availableCount;
	}
}

class CE_Spawner
{
	protected CE_ItemSpawningComponent SpawningComponent;
	protected CE_Item ItemSpawned;
	protected bool ReadyForItem;
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawner(CE_ItemSpawningComponent spawningComponent, CE_Item itemSpawned, bool readyForItem)
	{
		SpawningComponent = spawningComponent;
		ItemSpawned = itemSpawned;
		ReadyForItem = readyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_ItemSpawningComponent GetSpawningComponent()
	{
		return SpawningComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetSpawningComponent(CE_ItemSpawningComponent spawningComponent)
	{
		SpawningComponent = spawningComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_Item GetItemSpawned()
	{
		return ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetItemSpawned(CE_Item itemSpawned)
	{
		ItemSpawned = itemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool IsReadyForItem()
	{
		return ReadyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetReadyForItem(bool readyForItem)
	{
		ReadyForItem = readyForItem;
	}
}

class CE_SearchableContainer
{
	protected CE_SearchableContainerComponent ContainerComponent;
	protected array<CE_Item> ItemsSpawned;
	protected bool ReadyForItems;
	
	//------------------------------------------------------------------------------------------------
	void CE_SearchableContainer(CE_SearchableContainerComponent containerComponent, array<CE_Item> itemsSpawned, bool readyForItems)
	{
		ContainerComponent = containerComponent;
		ItemsSpawned = itemsSpawned;
		ReadyForItems = readyForItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	CE_SearchableContainerComponent GetContainerComponent()
	{
		return ContainerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetContainerComponent(CE_SearchableContainerComponent containerComponent)
	{
		ContainerComponent = containerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	array<CE_Item> GetItemsSpawned()
	{
		return ItemsSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool IsReadyForItems()
	{
		return ReadyForItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetReadyForItems(bool readyForItems)
	{
		ReadyForItems = readyForItems;
	}
}

void CE_ItemSpawned(IEntity itemEntity, CE_Item item);
typedef func CE_ItemSpawned;
typedef ScriptInvokerBase<CE_ItemSpawned> CE_OnItemSpawnedInvoker;

void CE_ItemDespawned(IEntity itemEntity, CE_Item item);
typedef func CE_ItemDespawned;
typedef ScriptInvokerBase<CE_ItemDespawned> CE_OnItemDespawnedInvoker;

void CE_VehicleTaken(IEntity vehicleEntity, CE_Item item);
typedef func CE_VehicleTaken;
typedef ScriptInvokerBase<CE_VehicleTaken> CE_OnVehicleTakenInvoker;

void CE_SpawnerReset(CE_ItemSpawningComponent spawner);
typedef func CE_SpawnerReset;
typedef ScriptInvokerBase<CE_SpawnerReset> CE_OnSpawnerResetInvoker;

void CE_ContainerSearched(IEntity containerEntity, CE_SearchableContainerComponent container, IEntity searcher);
typedef func CE_ContainerSearched;
typedef ScriptInvokerBase<CE_ContainerSearched> CE_OnContainerSearchedInvoker;

void CE_ContainerReset(IEntity containerEntity, CE_SearchableContainerComponent container);
typedef func CE_ContainerReset;
typedef ScriptInvokerBase<CE_ContainerReset> CE_OnContainerResetInvoker;
