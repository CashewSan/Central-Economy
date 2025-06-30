void CE_ItemSpawned(IEntity itemEntity, CE_ItemData itemData);
typedef func CE_ItemSpawned;
typedef ScriptInvokerBase<CE_ItemSpawned> CE_OnItemSpawnedInvoker;

void CE_ItemDespawned(IEntity itemEntity, CE_ItemData itemData);
typedef func CE_ItemDespawned;
typedef ScriptInvokerBase<CE_ItemDespawned> CE_OnItemDespawnedInvoker;

void CE_VehicleTaken(IEntity vehicleEntity, CE_ItemData itemData);
typedef func CE_VehicleTaken;
typedef ScriptInvokerBase<CE_VehicleTaken> CE_OnVehicleTakenInvoker;

void CE_SpawnerReset(CE_ItemSpawningComponent spawner);
typedef func CE_SpawnerReset;
typedef ScriptInvokerBase<CE_SpawnerReset> CE_OnSpawnerResetInvoker;

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
	
<<<<<<< Updated upstream
	[Attribute("1800000", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", category: "Item Data")] // default set to 1800000 seconds (30 minutes)
=======
	[Attribute("1800", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", params: "10 inf 10", category: "Item Data")] // default set to 1800 seconds (30 minutes) 
>>>>>>> Stashed changes
	int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
		
	protected bool 										m_bHasItemSpawned 						= false;								// has item spawned on the spawner?
	protected bool 										m_bWasItemDespawned 						= false;								// was the item despawned by the system?
	protected bool 										m_bHasItemBeenTaken 						= false;								// has item been taken from spawner?
	protected bool										m_bHasSpawnerReset						= true;								// has the spawner reset?
	
	protected int										m_iCurrentSpawnerResetTime				= 0;									// current spawner reset time
	
	protected string 									m_sSpawnedEntityUID;															// UID of the spawned entity
	
	protected ref CE_OnItemSpawnedInvoker 					m_OnItemSpawnedInvoker 					= new CE_OnItemSpawnedInvoker();
	protected ref CE_OnItemDespawnedInvoker 				m_OnItemDespawnedInvoker 					= new CE_OnItemDespawnedInvoker();
	protected ref CE_OnSpawnerResetInvoker 				m_OnSpawnerResetInvoker 					= new CE_OnSpawnerResetInvoker();
	protected ref CE_OnVehicleTakenInvoker					m_OnVehicleTakenInvoker					= new CE_OnVehicleTakenInvoker();
	
	protected ref array<ref CE_ItemData>					m_aItemData								= new array<ref CE_ItemData>;			// CE_ItemData array, item data gets inserted from config item data
	protected CE_ItemData 								m_ItemSpawned;																// item that has spawned on the spawner
	protected IEntity 									m_EntitySpawned;																// entity that has spawned on the spawner
	protected CE_ItemDataConfig							m_Config;																	// the set config for spawner, can be unique config or universal config
	protected CE_ItemSpawningSystem 						m_SpawningSystem;															// the spawning game system used to control spawning
	protected CE_SpawnerTimingSystem 						m_TimingSystem;																// the timing game system used to control spawner reset

	//------------------------------------------------------------------------------------------------
	//! Calls ConnectToItemSpawningSystem(), and sets default attributes if not set by trigger entities
	protected override void OnPostInit(IEntity owner)
	{
		HookEvents();
		
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
	
	void Update(int checkInterval)
	{
		if (HasItemBeenTaken())
		{
			SetCurrentSpawnerResetTime(Math.ClampInt(GetCurrentSpawnerResetTime() - checkInterval, 0, GetSpawnerResetTime()));
			if (GetCurrentSpawnerResetTime() == 0 && !HasSpawnerReset())
			{
<<<<<<< Updated upstream
				SetHasSpawnerReset(true);
				
				OnSpawnerReset();
			}
			
			//Print("Spawner Reset: " + GetCurrentSpawnerResetTime());
		}
		
		if (m_EntitySpawned)
		{
			CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(m_EntitySpawned.FindComponent(CE_ItemSpawnableComponent));
			if (itemSpawnable)
			{
				if (itemSpawnable.HasLifetimeEnded())
				{
					SCR_EntityHelper.DeleteEntityAndChildren(itemSpawnable.GetOwner());
					
					SetWasItemDespawned(true);
				}
			}
			else if (GetItemSpawned() && GetItemSpawned().m_sName)
			{
				Print("[CentralEconomy] THIS ENTITY HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + GetItemSpawned().m_sName, LogLevel.ERROR);
=======
				m_OnSpawnerResetInvoker.Invoke(this);
>>>>>>> Stashed changes
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
		
		GetGame().GetCallqueue().CallLater(LoadConfig, 100);
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
	//! Loads CE_ItemData config containing items, deciding if it's one attached to the component or the universal config
	protected void LoadConfig()
	{
		if (m_ItemDataConfig)
		{
			m_Config = m_ItemDataConfig;
			
			if (m_Config)
			{
				foreach (CE_ItemData itemData : m_Config.m_ItemData)
				{
					m_aItemData.Insert(itemData);
				}
			}
		}
<<<<<<< Updated upstream
		else
		{
			if(GetGame().InPlayMode())
			{
				CE_WorldValidationComponent m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
				
				if (m_WorldValidationComponent)
				{
					if(m_WorldValidationComponent.HasWorldProcessed())
					{	
						m_Config = m_WorldValidationComponent.GetItemDataConfig();
						
						if (m_Config)
						{
							foreach (CE_ItemData itemData : m_Config.m_ItemData)
							{
								m_aItemData.Insert(itemData);
							}
						}
						
						//Print("Config Item Count: " + m_Config.m_ItemData.Count());
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
		}
=======
>>>>>>> Stashed changes
		
		if (!m_Config)
			Print("[CentralEconomy::CE_ItemSpawningComponent] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Requests item for spawning (gets called from spawning system)
	CE_ItemData RequestItemToSpawn()
	{
		return GetItemToSpawn(m_aItemData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to item spawning system to request item from itemDataArray use the attributes from this component
	protected CE_ItemData GetItemToSpawn(array<ref CE_ItemData> itemDataArray)
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return null;

<<<<<<< Updated upstream
		return m_SpawningSystem.GetItem(itemDataArray, m_Tier, m_Usage, m_Categories);
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Tries to spawn the item (gets called from item spawning system)
	void TryToSpawnItem(CE_ItemData item)
	{
		if (!Replication.IsServer())
			return;
		
		if (HasItemSpawned())
		{		
			return;
		}
		else if (item)
		{
			SetItemSpawned(item);
			
			IEntity spawnEntity = GetOwner();
			if (!spawnEntity)
				return;
		
			vector m_WorldTransform[4];
			spawnEntity.GetWorldTransform(m_WorldTransform);
			
			EntitySpawnParams params();
			m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.200;
			params.Transform = m_WorldTransform;
			
			Resource m_Resource = Resource.Load(item.m_sPrefab);
			if (!m_Resource)
				return;
			
			IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
			if (!newEnt)
				return;
			
			newEnt.SetYawPitchRoll(item.m_vItemRotation + spawnEntity.GetYawPitchRoll());
			SCR_EntityHelper.SnapToGround(newEnt);
			
			SetIsNewSpawn(true);
			
			SetItemQuantity(newEnt, item.m_iQuantityMinimum, item.m_iQuantityMaximum);
			
			m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
			if (m_SpawningSystem)
			{
				m_SpawningSystem.GetSpawnedItems().Insert(item);
				m_SpawningSystem.ResetStallTimer();
				m_SpawningSystem.SetSpawnedItemCount(m_SpawningSystem.GetSpawnedItemCount() + 1);
				m_SpawningSystem.GetComponentsWithoutItem().RemoveItem(this);
			}
			
			// if item is a vehicle, or at least categorized as one (vehicles don't have a hierarchy component, so adding as child doesn't work)
			if (item.m_ItemCategory == CE_ELootCategory.VEHICLES
			|| item.m_ItemCategory == CE_ELootCategory.VEHICLES_SEA
			|| item.m_ItemCategory == CE_ELootCategory.VEHICLES_AIR)
			{
				// Vehicle enter/leave event
		        EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(newEnt.FindComponent(EventHandlerManagerComponent));
		        if (eventHandler)
		        {
		           	eventHandler.RegisterScriptHandler("OnCompartmentEntered", this, OnVehicleActivated, true, false);
					eventHandler.RegisterScriptHandler("OnCompartmentLeft", this, OnVehicleDeactivated, true, false);
					OnItemSpawned(newEnt);
		        }
			}
			else
			{
				spawnEntity.AddChild(newEnt, -1, EAddChildFlags.NONE);
			}
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
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Called when child (spawned item) is attached to spawning entity, calls OnItemSpawned()
	override void OnChildAdded(IEntity parent, IEntity child)
	{
		OnItemSpawned(child);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when child (spawned item) is detached from spawning entity, calls OnItemTaken()
	override void OnChildRemoved(IEntity parent, IEntity child)
	{
		OnItemTaken(child);
	}
=======
		return m_SpawningSystem.GetItemFromSpawnerConfig(itemDataArray, m_Tier, m_Usage, m_Categories);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to item spawning system to request item from the UNIVERSAL itemData config and uses the attributes from this component
	CE_ItemData GetItemToSpawnFromUniversalConfig()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return null;

		return m_SpawningSystem.GetItemFromUniversalConfig(m_Tier, m_Usage, m_Categories);
	}	
>>>>>>> Stashed changes
	
	//------------------------------------------------------------------------------------------------
	//! When spawned vehicle compartment is entered by player, adds DeferredVehicleRespawnCheck() to call queue and delays it by 10 seconds
	void OnVehicleActivated(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		GetGame().GetCallqueue().CallLater(DeferredVehicleRespawnCheck, 10000, false, vehicle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! When spawned vehicle compartment is left by player, removes DeferredVehicleRespawnCheck() from call queue
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
	//! Called when the item spawns or is attached as a child to the spawner entity, set various checks and balances
	protected void OnItemSpawned(IEntity itemEntity, CE_ItemData itemData)
	{
		//string itemUID = m_UIDGen.Generate();
		
		CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(itemEntity.FindComponent(CE_ItemSpawnableComponent));
		if (itemSpawnable)
		{
			//itemSpawnable.SetItemUID(itemUID);
			itemSpawnable.SetWasSpawnedBySystem(true);
			itemSpawnable.SetItemDataName(itemData.m_sName);
			itemSpawnable.SetRestockTime(itemData.m_iRestock);
			itemSpawnable.SetCurrentRestockTime(itemData.m_iRestock);
			itemSpawnable.SetLifetime(itemData.m_iLifetime);
			itemSpawnable.SetCurrentLifetime(itemData.m_iLifetime);
			itemSpawnable.SetItemDataCategory(itemData.m_ItemCategory);
		}
		else
			Print("[CentralEconomy::CE_ItemSpawningComponent] THIS ENTITY HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + itemData.m_sName, LogLevel.ERROR);
		
		SetItemSpawned(itemData);
		SetCurrentSpawnerResetTime(m_iSpawnerResetTime);
		SetEntitySpawned(itemEntity);
		//SetSpawnedEntityUID(itemUID);
		SetHasItemSpawned(true);
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
	//! Called when the item is taken or is detached as a child to the spawner entity, set various checks and balances
	protected void OnItemTaken(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		Print("Item Taken");
		
		SetHasItemBeenTaken(true);
		//SetSpawnedEntityUID(string.Empty);
		//SetCurrentSpawnerResetTime(m_iSpawnerResetTime);
		
		GetGame().GetCallqueue().CallLater(RemoveItemSpawnedFromSystem, 100, false, GetItemSpawned());
		
		CE_ItemSpawnableComponent itemSpawnable = CE_ItemSpawnableComponent.Cast(GetEntitySpawned().FindComponent(CE_ItemSpawnableComponent));
		if (itemSpawnable)
		{
			itemSpawnable.SetWasItemTaken(true);
			itemSpawnable.SetLifetime(0);
		}
		else
			Print("[CentralEconomy::CE_ItemSpawningComponent] THIS ENTITY HAS NO CE_ITEMSPAWNABLECOMPONENT!: " + GetItemSpawned().m_sName, LogLevel.ERROR);
		
		if (m_SpawningSystem)
		{
			m_SpawningSystem.GetItemsNotRestockReady().Insert(GetItemSpawned(), GetItemSpawned().m_sName);
		}
		
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
		SetHasItemSpawned(false);
		
		SetHasSpawnerReset(true);
			
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (m_SpawningSystem)
		{
			m_SpawningSystem.GetComponentsWithoutItem().Insert(this);
			m_SpawningSystem.SetSpawnedItemCount(m_SpawningSystem.GetSpawnedItemCount() - 1);
		}
		
		m_TimingSystem = CE_SpawnerTimingSystem.GetInstance();
		if (m_TimingSystem)
		{
			m_TimingSystem.UnregisterSpawner(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes item spawned from item spawning system
	protected void RemoveItemSpawnedFromSystem(CE_ItemData item)
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (m_SpawningSystem)
		{
			m_SpawningSystem.GetSpawnedItems().RemoveItemOrdered(item);
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
	//! Does the spawner have a item spawned on it?
	bool HasItemSpawned()
	{
		return m_bHasItemSpawned;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner has a item spawned on it
	void SetHasItemSpawned(bool spawned)
	{
		m_bHasItemSpawned = spawned;
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
	CE_ItemData GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the item spawned on the spawning component
	void SetItemSpawned(CE_ItemData item)
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
<<<<<<< Updated upstream
	//! Is the item a new spawn?
	bool IsNewSpawn()
	{
		return m_bIsNewSpawn;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item is a new spawn
	void SetIsNewSpawn(bool spawn)
	{
		m_bIsNewSpawn = spawn;
=======
	//! Does this spawner have a config?
	bool HasSpawnerConfig()
	{
		return m_bHasSpawnerConfig;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner has a config
	void SetHasSpawnerConfig(bool config)
	{
		m_bHasSpawnerConfig = config;
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
>>>>>>> Stashed changes
	}
}