[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to be added to spawner entities to handle item spawning")]
class CE_ItemSpawningComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawningComponent : ScriptComponent
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used (If set, universal config through CE_WorldValidationComponent will be ignored!)", "conf", category: "Spawner Data")]
	protected ref CE_ItemDataConfig m_ItemDataConfig;
	
	[Attribute("", UIWidgets.ComboBox, desc: "Which spawn location(s) will this spawner be? (If set, usages set throughout the world will be ignored!)", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Spawner Data")]
	protected CE_ELootUsage m_ItemUsage;
	
	[Attribute("", UIWidgets.Flags, desc: "Which category of items do you want to spawn here?", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Spawner Data")]
	protected CE_ELootCategory m_Categories;
	
	[Attribute("1800", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", params: "10 inf 10", category: "Spawner Data")] // default set to 1800 seconds (30 minutes)
	protected int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; 																			// what is the usage of this spawner component? Can be either m_ItemUsage or set throughout the world
	protected CE_ELootTier 								m_Tier; 																				// what is the tier of this spawner component?
	
	protected bool 										m_bHasConfig 							= false;										// does the spawner have a config set in the component?
	protected bool 										m_bReadyForItem 							= true;										// has item spawned on the spawner?
	protected bool										m_bHaveItemsProcessed	;																// have the items of the spawner been processed? ONLY APPLICABLE IF m_ItemDataConfig IS SET!
	protected bool										m_bHasUsage								= false;										// does the spawner have a usage set through the component?
	
	protected int										m_iCurrentSpawnerResetTime				= 0;											// current spawner reset time
	
	protected ref CE_OnItemSpawnedOnSpawnerInvoker 		m_OnItemSpawnedInvoker 					= new CE_OnItemSpawnedOnSpawnerInvoker();		// script invoker for when a item has spawned on the spawner
	protected ref CE_OnItemDespawnedFromSpawnerInvoker 	m_OnItemDespawnedInvoker 					= new CE_OnItemDespawnedFromSpawnerInvoker();	// script invoker for when a item has despawned from the spawner
	protected ref CE_OnSpawnerResetInvoker 				m_OnSpawnerResetInvoker 					= new CE_OnSpawnerResetInvoker();				// script invoker for when a spawner has been reset
	
	protected ref array<ref CE_Item>						m_aItems;																			// CE_Item array, items processed from system IF the spawner has it's own config set (m_ItemDataConfig)
	
	protected ref CE_Item 								m_ItemSpawned;																		// CE_Item that has spawned on the spawner
	protected IEntity 									m_EntitySpawned;																		// IEntity that has spawned on the spawner
	protected CE_ItemSpawningSystem 						m_SpawningSystem;																	// item spawning system that handles all item spawning with CentralEconomy (A.K.A. the brain)
	protected CE_SpawnerTimingSystem 						m_TimingSystem;																		// spawner timing system that handles all spawner reset timings
	
	//------------------------------------------------------------------------------------------------
	//! Post initialization
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		const RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (rplComp && rplComp.IsProxy())
		{
			return;
		}
		
		HookEvents();
		
		if (m_ItemUsage)
		{
			m_bHasUsage = true;
			m_Usage = m_ItemUsage;
		}
		
		if (m_ItemDataConfig)
			LoadConfig();
		else
			ConnectToItemSpawningSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called from the m_TimingSystem to control spawner reset
	void Update(int checkInterval)
	{
		m_iCurrentSpawnerResetTime = Math.ClampInt(GetCurrentSpawnerResetTime() - checkInterval, 0, GetSpawnerResetTime());
		if (GetCurrentSpawnerResetTime() == 0)
		{
			m_OnSpawnerResetInvoker.Invoke(this);
		}
		
		//Print("Timer: " + GetCurrentSpawnerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize necessary callbacks
	protected void HookEvents()
	{
		m_OnItemSpawnedInvoker.Insert(OnItemSpawned);
		m_OnItemDespawnedInvoker.Insert(OnItemDespawned);
		m_OnSpawnerResetInvoker.Insert(OnSpawnerReset);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this spawner
	void ConnectToItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(GetOwner());
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterSpawner(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this spawner
	protected void DisconnectFromItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(GetOwner());
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterSpawner(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads m_ItemDataConfig, if it exists, and calls to process the item data from it
	protected void LoadConfig()
	{
		if (m_ItemDataConfig)
		{
			m_aItems	= new array<ref CE_Item>;

			m_bHasConfig = true;
			
			m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(GetOwner());
			if (!m_SpawningSystem)
				return;
			
			foreach (CE_Item item : m_SpawningSystem.ProcessItemData(m_ItemDataConfig))
			{
				m_aItems.Insert(item);
			}
			
			m_bHaveItemsProcessed = true;
		}
		else
			Print("[CentralEconomy::CE_ItemSpawningComponent] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR); // this should realistically never happen
		
		ConnectToItemSpawningSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! When spawned vehicle compartment is entered by player
	void OnVehicleActivated(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		GetGame().GetCallqueue().CallLater(DeferredVehicleRespawnCheck, 10000, false, vehicle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! When spawned vehicle compartment is left by player
	void OnVehicleDeactivated(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		GetGame().GetCallqueue().Remove(DeferredVehicleRespawnCheck);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if vehicle has travelled far enough from original spawn point to be considered "taken" from spawner
	protected void DeferredVehicleRespawnCheck(IEntity vehicle)
	{
		const float VEHICLE_RESPAWN_DISTANCE_THRESHOLD = 2000.0;
		
		float distance = vector.DistanceSq(vehicle.GetOrigin(), GetOwner().GetOrigin());
		
		//PrintFormat("DeferredVehicleRespawnCheck(%1, %2): Distance from spawn: %3", this, vehicle, distance);
		if (distance > VEHICLE_RESPAWN_DISTANCE_THRESHOLD)
		{
			// vehicle has moved away from spawn so enable respawning it
			//PrintFormat("DeferredVehicleRespawnCheck(%1, %2): Enabling respawn", this, vehicle);
			
			InventoryItemComponent itemComponent = InventoryItemComponent.Cast(vehicle.FindComponent(InventoryItemComponent));
			if (itemComponent)
				itemComponent.m_OnParentSlotChangedInvoker.Invoke(null, null); // slots don't apply with vehicles so not necessary (I think)
		}
		else
		{
			// if vehicle hasn't moved far enough, do another check until it has moved far enough
			GetGame().GetCallqueue().CallLater(DeferredVehicleRespawnCheck, 10000, false, vehicle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item spawns
	protected void OnItemSpawned(IEntity itemEntity, CE_Item item, CE_Spawner spawner)
	{
		CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(itemEntity.FindComponent(CE_ItemSpawnableComponent));
		if (itemSpawnable)
		{
			itemSpawnable.HookSpawningEvents();
			itemSpawnable.GetItemSpawnedInvoker().Invoke(item, spawner);
		}
		else
		{
			CE_ItemData itemData = item.GetItemData();
			if (itemData)
			{
				Print("[CentralEconomy::CE_ItemSpawningComponent] THIS ITEM HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + itemData.GetName(), LogLevel.ERROR);
			}
		}
		
		m_bReadyForItem = false;
		m_ItemSpawned = item;
		m_iCurrentSpawnerResetTime = m_iSpawnerResetTime;
		m_EntitySpawned = itemEntity;
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnItemTaken);
		
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(GetOwner());
		if (m_SpawningSystem)
		{
			m_SpawningSystem.UnregisterSpawner(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item despawns
	protected void OnItemDespawned(IEntity itemEntity, CE_ItemData itemData)
	{
		m_OnSpawnerResetInvoker.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item is taken
	protected void OnItemTaken(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		m_TimingSystem = CE_SpawnerTimingSystem.GetByEntityWorld(GetOwner());
		if (m_TimingSystem)
		{
			m_TimingSystem.RegisterSpawner(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when spawner resets
	protected void OnSpawnerReset(CE_ItemSpawningComponent spawner)
	{
		m_EntitySpawned = null;
		m_ItemSpawned = null;
		m_bReadyForItem = true;
		
		m_TimingSystem = CE_SpawnerTimingSystem.GetByEntityWorld(GetOwner());
		if (m_TimingSystem)
		{
			m_TimingSystem.UnregisterSpawner(this);
		}
		
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(GetOwner());
		if (m_SpawningSystem)
		{
			m_SpawningSystem.RegisterSpawner(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// COMPONENT STUFF
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Calls for disconnect from item spawning system on deletion of component
	override void OnDelete(IEntity owner)
	{
		DisconnectFromItemSpawningSystem();

		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Is the spawner ready for an item to be spawned on it?
	bool IsReadyForItem()
	{
		return m_bReadyForItem;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner is ready for an item to be spawned on it
	void SetReadyForItem(bool ready)
	{
		m_bReadyForItem = ready;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets CE_Item spawned on the spawning component
	CE_Item GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_Item spawned on the spawning component
	void SetItemSpawned(CE_Item item)
	{
		m_ItemSpawned = item;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the IEntity that spawned on the spawner (different from GetItemSpawned())
	IEntity GetEntitySpawned()
	{
		return m_EntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the IEntity that spawned on the spawner (different from SetItemSpawned())
	void SetEntitySpawned(IEntity ent)
	{
		m_EntitySpawned = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the CE_ELootTier of the spawner
	CE_ELootTier GetSpawnerTier()
	{
		return m_Tier;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootTier of the spawner
	void SetSpawnerTier(CE_ELootTier tier)
	{
		m_Tier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the CE_ELootUsage of the spawner
	CE_ELootUsage GetSpawnerUsage()
	{
		return m_Usage;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootUsage of the spawner
	void SetSpawnerUsage(CE_ELootUsage usage)
	{
		m_Usage = usage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the CE_ELootCategory(s) of the spawner
	CE_ELootCategory GetSpawnerCategories()
	{
		return m_Categories;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootCategory(s) of the spawner
	void SetSpawnerCategories(CE_ELootCategory category)
	{
		m_Categories = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the current spawner reset time
	int GetCurrentSpawnerResetTime()
	{
		return m_iCurrentSpawnerResetTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the current spawner reset time
	void SetCurrentSpawnerResetTime(int time)
	{
		m_iCurrentSpawnerResetTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the total spawner reset time
	int GetSpawnerResetTime()
	{
		return m_iSpawnerResetTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does this spawner have a config set by the component?
	bool HasConfig()
	{
		return m_bHasConfig;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner has a config set by the component
	void SetHasConfig(bool config)
	{
		m_bHasConfig = config;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does this spawner have a usage set by the component?
	bool HasUsage()
	{
		return m_bHasUsage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Have items of this spawner been processed (only applies if spawner has it's own config set, will return null otherwise)
	bool HaveItemsProcessed()
	{
		return m_bHaveItemsProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if items of this spawner have been processed (only applies if spawner has it's own config set)
	void SetHaveItemsProcessed(bool processed)
	{
		m_bHaveItemsProcessed = processed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the spawners item data config, if set (if NOT set, will return null)
	CE_ItemDataConfig GetConfig()
	{
		return m_ItemDataConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the container's item data config, if set (if NOT set, will return null)
	array<ref CE_Item> GetItems()
	{
		return m_aItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item spawned script invoker
	CE_OnItemSpawnedOnSpawnerInvoker GetItemSpawnedInvoker()
	{
		return m_OnItemSpawnedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item despawned script invoker
	CE_OnItemDespawnedFromSpawnerInvoker GetItemDespawnedInvoker()
	{
		return m_OnItemDespawnedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawner reset script invoker
	CE_OnSpawnerResetInvoker GetSpawnerResetnvoker()
	{
		return m_OnSpawnerResetInvoker;
	}
}