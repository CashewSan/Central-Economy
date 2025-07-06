[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to be added to spawner entities to handle item spawning")]
class CE_ItemSpawningComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawningComponent : ScriptComponent
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used (If set, universal config through CE_WorldValidationComponent will be ignored!)", "conf", category: "Item Data")]
	ref CE_ItemDataConfig m_ItemDataConfig;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which spawn location(s) will this item spawn in? (If set, universal usages set throughout the world will be ignored!)", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Item Data")]
	CE_ELootUsage m_ItemUsage;
	
	[Attribute(ResourceName.Empty, UIWidgets.Flags, desc: "Category of loot spawn", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Item Data")]
	CE_ELootCategory m_Categories;
	
	[Attribute("1800", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", params: "10 inf 10", category: "Item Data")] // default set to 1800 seconds (30 minutes) 
	int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
		
	protected bool 										m_bHasConfig 							= false;								// does the spawner have a config set in the component?
	protected bool 										m_bWasItemDespawned 						= false;								// was the item despawned by the system?
	protected bool 										m_bHasItemBeenTaken 						= false;								// has item been taken from spawner?
	protected bool										m_bHasSpawnerReset						= true;								// has the spawner reset?
	protected bool 										m_bReadyForItem 							= true;								// has item spawned on the spawner?
	
	protected int										m_iCurrentSpawnerResetTime				= 0;									// current spawner reset time
	
	protected string 									m_sSpawnedEntityUID;															// UID of the spawned entity
	
	protected ref CE_OnItemSpawnedInvoker 					m_OnItemSpawnedInvoker 					= new CE_OnItemSpawnedInvoker();
	protected ref CE_OnItemDespawnedInvoker 				m_OnItemDespawnedInvoker 					= new CE_OnItemDespawnedInvoker();
	protected ref CE_OnSpawnerResetInvoker 				m_OnSpawnerResetInvoker 					= new CE_OnSpawnerResetInvoker();
	protected ref CE_OnVehicleTakenInvoker					m_OnVehicleTakenInvoker					= new CE_OnVehicleTakenInvoker();
	
	protected ref array<ref CE_ItemData>					m_aItemData								= new array<ref CE_ItemData>;			// CE_ItemData array, item data gets inserted from config item data
	protected ref CE_Item 								m_ItemSpawned;																// item that has spawned on the spawner
	protected IEntity 									m_EntitySpawned;																// entity that has spawned on the spawner
	protected CE_ItemDataConfig							m_Config;																	// the set config for spawner, can be unique config or universal config
	protected CE_ItemSpawningSystem 						m_SpawningSystem;															// the spawning game system used to control spawning
	protected CE_SpawnerTimingSystem 						m_TimingSystem;																// the timing game system used to control spawner reset

	//------------------------------------------------------------------------------------------------
	//! 
	protected override void OnPostInit(IEntity owner)
	{
		HookEvents();
		
		if (m_ItemDataConfig)
			LoadConfig();
		
		GetGame().GetCallqueue().CallLater(ConnectToItemSpawningSystem, 500);
		
		// Defaults for if no info gets set
		if (!m_Tier)
			m_Tier = CE_ELootTier.TIER1;
		
		if (m_ItemUsage)
		{
			m_Usage = m_ItemUsage;
		}
		else if (!m_Usage)
			m_Usage = CE_ELootUsage.TOWN;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Update(int checkInterval)
	{
		if (HasItemBeenTaken())
		{
			SetCurrentSpawnerResetTime(Math.ClampInt(GetCurrentSpawnerResetTime() - checkInterval, 0, GetSpawnerResetTime()));
			if (GetCurrentSpawnerResetTime() == 0 && !HasSpawnerReset())
			{
				m_OnSpawnerResetInvoker.Invoke(this);
			}
		}
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
	//! Connects to item spawning system and registers this component
	protected void ConnectToItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterSpawner(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterSpawner(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	protected void LoadConfig()
	{
		m_Config = m_ItemDataConfig;
		
		if (m_Config)
		{
			SetHasConfig(true);
		}
		
		if (!m_Config)
			Print("[CentralEconomy::CE_ItemSpawningComponent] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR);
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
	protected void OnItemSpawned(IEntity itemEntity, CE_Item item)
	{
		CE_ItemData itemData = item.GetItemData();
		if (!itemData)
			return;
		
		CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(itemEntity.FindComponent(CE_ItemSpawnableComponent));
		if (itemSpawnable)
		{
			itemSpawnable.SetItem(item);
			itemSpawnable.SetWasSpawnedBySystem(true);
			itemSpawnable.SetTotalRestockTime(itemData.m_iRestock);
			itemSpawnable.SetCurrentRestockTime(itemData.m_iRestock);
			itemSpawnable.SetTotalLifetime(itemData.m_iLifetime);
			itemSpawnable.SetCurrentLifetime(itemData.m_iLifetime);
		}
		else
			Print("[CentralEconomy::CE_ItemSpawningComponent] THIS ENTITY HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + itemData.m_sName, LogLevel.ERROR);
		
		SetReadyForItem(false);
		SetItemSpawned(item);
		SetCurrentSpawnerResetTime(m_iSpawnerResetTime);
		SetEntitySpawned(itemEntity);
		SetWasItemDespawned(false);
		SetHasItemBeenTaken(false);
		SetHasSpawnerReset(false);
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnItemTaken);
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
		Print("Item Taken");
		
		SetHasItemBeenTaken(true);
		
		CE_ItemData itemData = GetItemSpawned().GetItemData();
		
		CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(GetEntitySpawned().FindComponent(CE_ItemSpawnableComponent));
		if (itemSpawnable)
		{
			itemSpawnable.SetWasItemTaken(true);
			itemSpawnable.SetTotalLifetime(0);
		}
		else
			Print("[CentralEconomy::CE_ItemSpawningComponent] THIS ENTITY HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + itemData.m_sName, LogLevel.ERROR);
		
		m_TimingSystem = CE_SpawnerTimingSystem.GetInstance();
		if (m_TimingSystem)
		{
			m_TimingSystem.RegisterSpawner(this);
		}
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetEntitySpawned().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Remove(OnItemTaken);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when spawner resets
	protected void OnSpawnerReset(CE_ItemSpawningComponent spawner)
	{
		//Print("Spawner Reset");
		
		//SetSpawnedEntityUID(string.Empty);
		SetEntitySpawned(null);
		SetReadyForItem(true);
		
		SetHasSpawnerReset(true);
			
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (m_SpawningSystem)
		{
			//m_SpawningSystem.GetComponentsWithItem().RemoveItem(this);
			//m_SpawningSystem.GetComponentsWithoutItem().Insert(this);
			//m_SpawningSystem.SetSpawnedItemCount(m_SpawningSystem.GetSpawnedItemCount() - 1);
		}
		
		m_TimingSystem = CE_SpawnerTimingSystem.GetInstance();
		if (m_TimingSystem)
		{
			m_TimingSystem.UnregisterSpawner(this);
		}
	}
	
	// Component Stuff
	
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
	//! 
	bool IsReadyForItem()
	{
		return m_bReadyForItem;
	}
		
	//------------------------------------------------------------------------------------------------
	//! 
	void SetReadyForItem(bool ready)
	{
		m_bReadyForItem = ready;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was the item despawned from spawner? NOT TAKEN
	bool WasItemDespawned()
	{
		return m_bWasItemDespawned;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was despawned from the spawner
	void SetWasItemDespawned(bool despawned)
	{
		m_bWasItemDespawned = despawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has item been taken from the spawner? NOT DESPAWNED
	bool HasItemBeenTaken()
	{
		return m_bHasItemBeenTaken;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item has been taken from the spawner
	void SetHasItemBeenTaken(bool taken)
	{
		m_bHasItemBeenTaken = taken;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item spawned on the spawning component
	CE_Item GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the item spawned on the spawning component
	void SetItemSpawned(CE_Item item)
	{
		m_ItemSpawned = item;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets entity spawned on the spawning component (different from GetItemSpawned())
	IEntity GetEntitySpawned()
	{
		return m_EntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the entity spawned on the spawning component (different from SetItemSpawned())
	void SetEntitySpawned(IEntity ent)
	{
		m_EntitySpawned = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the tier of the spawning component
	CE_ELootTier GetSpawnerTier()
	{
		return m_Tier;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the tier of the spawning component
	void SetSpawnerTier(CE_ELootTier tier)
	{
		m_Tier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the usage of the spawning component
	CE_ELootUsage GetSpawnerUsage()
	{
		return m_Usage;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the usage of the spawning component
	void SetSpawnerUsage(CE_ELootUsage usage)
	{
		m_Usage = usage;
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
	//! Gets the spawner reset time
	int GetSpawnerResetTime()
	{
		return m_iSpawnerResetTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has the spawner reset?
	bool HasSpawnerReset()
	{
		return m_bHasSpawnerReset;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner has been reset
	void SetHasSpawnerReset(bool reset)
	{
		m_bHasSpawnerReset = reset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does this spawner have a config?
	bool HasConfig()
	{
		return m_bHasConfig;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner has a config
	void SetHasConfig(bool config)
	{
		m_bHasConfig = config;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the spawners item data config, if set (if NOT set, will return null)
	CE_ItemDataConfig GetConfig()
	{
		return m_Config;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item spawned script invoker
	CE_OnItemSpawnedInvoker GetItemSpawnedInvoker()
	{
		return m_OnItemSpawnedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item despawned script invoker
	CE_OnItemDespawnedInvoker GetItemDespawnedInvoker()
	{
		return m_OnItemDespawnedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawner reset script invoker
	CE_OnSpawnerResetInvoker GetSpawnerResetnvoker()
	{
		return m_OnSpawnerResetInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets vehicle taken script invoker
	CE_OnVehicleTakenInvoker GetVehicleTakenInvoker()
	{
		return m_OnVehicleTakenInvoker;
	}
}