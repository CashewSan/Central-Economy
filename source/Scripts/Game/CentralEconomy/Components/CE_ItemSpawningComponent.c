// Big thanks to Wyqydsyq for help with handling the vehicle features <3

[ComponentEditorProps(category: "CentralEconomy/Components", description: "")]
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
	
	[Attribute("1800000", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", category: "Item Data")] // default set to 1800000 seconds (30 minutes)
	int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
		
	protected bool 										m_bHasItemSpawned 						= false;								// has item spawned on the spawner?
	protected bool 										m_bWasItemDespawned 						= false;								// was the item despawned by the system?
	protected bool 										m_bHasItemBeenTaken 						= false;								// has item been taken from spawner?
	protected bool										m_bHasItemRestockEnded					= false;								// has item restock timer ended?
	
	protected ref array<ref CE_ItemData>					m_aItemData								= new array<ref CE_ItemData>;			// CE_ItemData array, item data gets inserted from config item data
	
	protected CE_ItemData 								m_ItemChosen;																// item chosen to spawn
	protected CE_ItemData 								m_ItemSpawned;																// item that has spawned on the spawner
	
	protected CE_ItemDataConfig							m_Config;																	// the set config for spawner, can be unique config or universal config
	
	protected CE_ItemSpawningSystem 						m_SpawningSystem;															// the spawning game system used to control spawning

	//------------------------------------------------------------------------------------------------
	void CE_ItemSpawningComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		ConnectToLootSpawningSystem();
		
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
	protected void ConnectToLootSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.Register(this);
		
		GetGame().GetCallqueue().CallLater(LoadConfig, 100);
	}
	
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
		else
		{
			if(GetGame().InPlayMode())
			{
				CE_WorldValidationComponent m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
				
				if (m_WorldValidationComponent)
				{
					if(m_WorldValidationComponent.GetWorldProcessed())
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
		
		if (!m_Config)
			Print("[CentralEconomy] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromLootSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData RequestItemToSpawn()
	{
		return GetItemToSpawn(m_aItemData);
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData GetItemToSpawn(array<ref CE_ItemData> itemDataArray)
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return null;

		return m_SpawningSystem.GetItem(itemDataArray, m_Tier, m_Usage, m_Categories);
	}
	
	//------------------------------------------------------------------------------------------------
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
			//Print("Asking To Spawn");
			
			IEntity spawnEntity = GetOwner();
		
			vector m_WorldTransform[4];
			spawnEntity.GetWorldTransform(m_WorldTransform);
			
			EntitySpawnParams params();
			m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.200;
			params.Transform = m_WorldTransform;
			
			Resource m_Resource = Resource.Load(item.m_sPrefab);
		
			IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
			
			newEnt.SetYawPitchRoll(item.m_vItemRotation + spawnEntity.GetYawPitchRoll());
			SCR_EntityHelper.SnapToGround(newEnt);
			
			SetItemQuantity(newEnt, item.m_iQuantityMinimum, item.m_iQuantityMaximum);
			
			SetItemSpawned(item);
			
			m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
			if (m_SpawningSystem)
			{
				if (!m_SpawningSystem.GetSpawnedItems().Contains(item))
					m_SpawningSystem.GetSpawnedItems().Insert(item);
				
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
	
	//------------------------------------------------------------------------------------------------
	override void OnChildAdded(IEntity parent, IEntity child)
	{
		OnItemSpawned(child);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnChildRemoved(IEntity parent, IEntity child)
	{
		OnItemTaken(child);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleActivated(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		GetGame().GetCallqueue().CallLater(DeferredVehicleRespawnCheck, 10000, false, vehicle);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDeactivated(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		GetGame().GetCallqueue().Remove(DeferredVehicleRespawnCheck);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeferredVehicleRespawnCheck(IEntity vehicle)
	{
		const float VEHICLE_RESPAWN_DISTANCE_THRESHOLD = 2000.0;
		
		float distance = vector.DistanceSq(vehicle.GetOrigin(), GetOwner().GetOrigin());
		
		//PrintFormat("MO: DeferredVehicleRespawnCheck(%1, %2): Distance from spawn: %3", this, vehicle, distance);
		if (distance > VEHICLE_RESPAWN_DISTANCE_THRESHOLD)
		{
			// vehicle has moved away from spawn so enable respawning it
			//PrintFormat("MO: DeferredVehicleRespawnCheck(%1, %2): Enabling respawn", this, vehicle);
			
			OnItemTaken(vehicle);
		}
		else
		{
			// if vehicle hasn't moved far enough, do another check until it has moved far enough
			GetGame().GetCallqueue().CallLater(DeferredVehicleRespawnCheck, 10000, false, vehicle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemSpawned(IEntity item)
	{
		SetHasItemSpawned(true);
		SetWasItemDespawned(false);
		SetHasItemRestockEnded(false);
		SetHasItemBeenTaken(false);
		
		if (!GetItemSpawned() || !GetItemSpawned().m_iLifetime)
			GetGame().GetCallqueue().CallLater(DespawnItem, 14400 * 1000, false, item);
		else
			GetGame().GetCallqueue().CallLater(DespawnItem, GetItemSpawned().m_iLifetime * 1000, false, item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemTaken(IEntity item)
	{
		SetHasItemBeenTaken(true);
		
		GetGame().GetCallqueue().CallLater(SpawnerReset, 100, false);
		GetGame().GetCallqueue().CallLater(RemoveItemSpawnedFromSystem, 100, false, GetItemSpawned());
		
		if (!GetItemSpawned() || !GetItemSpawned().m_iRestock)
			GetGame().GetCallqueue().CallLater(RestockReset, 3600 * 1000, false, GetItemSpawned()); // default restock
		else
			GetGame().GetCallqueue().CallLater(RestockReset, GetItemSpawned().m_iRestock * 1000, false, GetItemSpawned());
		
		GetGame().GetCallqueue().Remove(DespawnItem);
		
		if (m_SpawningSystem)
			m_SpawningSystem.GetItemsNotRestockReady().Insert(GetItemSpawned(), GetItemSpawned().m_sName);
	}
	
	//------------------------------------------------------------------------------------------------
	void DespawnItem(IEntity item)
	{
		if (item && !WasItemDespawned() && !HasItemBeenTaken())
		{
			SCR_EntityHelper.DeleteEntityAndChildren(item);
		
			SetWasItemDespawned(true);
			
			GetGame().GetCallqueue().CallLater(RemoveItemSpawnedFromSystem, 100, false, GetItemSpawned());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnerReset()
	{
		if (WasItemDespawned())
		{
			SetHasItemSpawned(false);
		}
		else
		{
			GetGame().GetCallqueue().CallLater(SetHasItemSpawned, m_iSpawnerResetTime, false, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RestockReset(CE_ItemData item)
	{
		SetHasItemRestockEnded(true);
		
		if (m_SpawningSystem)
			m_SpawningSystem.GetItemsNotRestockReady().Remove(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveItemSpawnedFromSystem(notnull CE_ItemData item)
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (m_SpawningSystem)
		{
			int index = m_SpawningSystem.GetSpawnedItems().Find(item);
			
			m_SpawningSystem.GetSpawnedItems().RemoveOrdered(index);
		}
	}
	
	// Component Stuff
	//------------------------------------------------------------------------------------------------
	void ~CE_ItemSpawningComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		DisconnectFromLootSpawningSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromLootSpawningSystem();

		super.OnDelete(owner);
	}
	
	// Getters/Setters
	//------------------------------------------------------------------------------------------------
	bool HasItemSpawned()
	{
		return m_bHasItemSpawned;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetHasItemSpawned(bool spawned)
	{
		m_bHasItemSpawned = spawned;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasItemRestockEnded()
	{
		return m_bHasItemRestockEnded;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetHasItemRestockEnded(bool ended)
	{
		m_bHasItemRestockEnded = ended;
	}
	
	//------------------------------------------------------------------------------------------------
	bool WasItemDespawned()
	{
		return m_bWasItemDespawned;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetWasItemDespawned(bool despawned)
	{
		m_bWasItemDespawned = despawned;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasItemBeenTaken()
	{
		return m_bHasItemBeenTaken;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetHasItemBeenTaken(bool taken)
	{
		m_bHasItemBeenTaken = taken;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemSpawned(notnull CE_ItemData item)
	{
		m_ItemSpawned = item;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ELootTier GetSpawnerTier()
	{
		return m_Tier;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetSpawnerTier(CE_ELootTier tier)
	{
		m_Tier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ELootUsage GetSpawnerUsage()
	{
		return m_Usage;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetSpawnerUsage(CE_ELootUsage usage)
	{
		m_Usage = usage;
	}
}