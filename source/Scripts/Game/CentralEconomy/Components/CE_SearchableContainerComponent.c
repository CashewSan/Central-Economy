void CE_ContainerSearched(IEntity containerEntity, CE_SearchableContainerComponent container, IEntity searcher);
typedef func CE_ContainerSearched;
typedef ScriptInvokerBase<CE_ContainerSearched> CE_OnContainerSearchedInvoker;

void CE_ContainerReset(IEntity containerEntity, CE_SearchableContainerComponent container);
typedef func CE_ContainerReset;
typedef ScriptInvokerBase<CE_ContainerReset> CE_OnContainerResetInvoker;

[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to searchable containers")]
class CE_SearchableContainerComponentClass : ScriptComponentClass
{
}

class CE_SearchableContainerComponent : ScriptComponent
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used (If set, universal config through CE_WorldValidationComponent will be ignored!)", "conf", category: "Container Data")]
	ref CE_ItemDataConfig m_ItemDataConfig;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which spawn location(s) will this container spawn items in? (If set, universal usages set throughout the world will be ignored!)", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Container Data")]
	CE_ELootUsage m_ContainerUsage;
	
	[Attribute(ResourceName.Empty, UIWidgets.Flags, desc: "Category of searchable container", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Container Data")]
	CE_ELootCategory m_Categories;
	
	[Attribute("1800", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the container to reset after it was initially searched. Helps prevent loot camping.", params: "10 inf 10", category: "Container Data")] // default set to 1800 seconds (30 minutes)
	int m_iContainerResetTime;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Minimum amount of items you want this searchable container to potentially spawn.", params: "0 inf 1", category: "Container Data")] // default set to 0
	int m_iContainerItemMinimum;
	
	[Attribute("3", UIWidgets.EditBox, desc: "Maximum amount of items you want this searchable container to potentially spawn.", params: "0 inf 1", category: "Container Data")] // default set to 3
	int m_iContainerItemMaximum;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
	
	protected bool 										m_bHasItemDataConfig 						= false;								// does the searchable container have a custom item data config? (NOT THE UNIVERSAL ONE)
	protected bool 										m_bHasBeenSearched						= false;								// has the container been searched?
	protected bool 										m_bWereItemsDespawned 					= false;								// was the items despawned by the system?
	protected bool										m_bHasContainerReset						= false;								// has the container reset?
	
	protected int										m_iCurrentContainerResetTime				= 0;									// current container reset time
	
	protected ref CE_OnContainerSearchedInvoker 			m_OnContainerSearchedInvoker 				= new CE_OnContainerSearchedInvoker();	// script invoker for when the container is searched
	protected ref CE_OnContainerResetInvoker 				m_OnContainerResetInvoker 				= new CE_OnContainerResetInvoker();	// script invoker for when the container is reset
	
	protected ref array<ref CE_ItemData>					m_aItemData								= new array<ref CE_ItemData>;			// CE_ItemData array, item data gets inserted from config item data
	protected CE_ItemDataConfig							m_Config;																	// the set config for container, can be unique config or universal config
	protected CE_ItemSpawningSystem 						m_SpawningSystem;															// the spawning game system used to control spawning
	protected CE_ContainerTimingSystem					m_TimingSystem;																// the timing system for controlling container reset
	
	//------------------------------------------------------------------------------------------------
	//! Sets default attributes if not set by trigger entities
	protected override void OnPostInit(IEntity owner)
	{
		owner.ClearFlags(EntityFlags.STATIC);
		
		HookEvents();
		
		// Defaults for if no info gets set
		if (!m_Tier)
			m_Tier = CE_ELootTier.TIER1;
		
		if (m_ContainerUsage)
		{
			m_Usage = m_ContainerUsage;
		}
		else if (!m_Usage)
			m_Usage = CE_ELootUsage.TOWN;
		
		SetCurrentContainerResetTime(GetContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize necessary callbacks
	protected void HookEvents()
	{
		m_OnContainerSearchedInvoker.Insert(OnContainerSearched);
		m_OnContainerResetInvoker.Insert(OnContainerReset);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method
	void Update(int checkInterval)
	{
		//Print("Container Reset Check");
		
		SetCurrentContainerResetTime(Math.ClampInt(GetCurrentContainerResetTime() - checkInterval, 0, GetContainerResetTime()));
		if (GetCurrentContainerResetTime() == 0 && HasBeenSearched())
		{
			m_OnContainerResetInvoker.Invoke(GetOwner(), this);
		}
		
		//Print("Timer: " + GetCurrentContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this component
	protected void ConnectToItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterContainer(this);
		
		if (m_ItemDataConfig)
			LoadConfig();
		// else we're gonna rely on universal config
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterContainer(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this component
	protected void ConnectToTimingSystem()
	{
		m_TimingSystem = CE_ContainerTimingSystem.GetInstance();
		if (!m_TimingSystem)
			return;
		
		m_TimingSystem.RegisterContainer(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromTimingSystem()
	{
		m_TimingSystem = CE_ContainerTimingSystem.GetInstance();
		if (!m_TimingSystem)
			return;

		m_TimingSystem.UnregisterContainer(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads CE_ItemData config containing items, deciding if it's one attached to the component or the universal config
	protected void LoadConfig()
	{
		m_Config = m_ItemDataConfig;
		
		if (m_Config)
		{
			SetHasItemDataConfig(true);
			
			foreach (CE_ItemData itemData : m_Config.m_ItemData)
			{
				m_aItemData.Insert(itemData);
			}
		}
		
		if (!m_Config)
			Print("[CentralEconomy::CE_SearchableContainerComponent] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to item spawning to request item
	array<CE_ItemData> GetItemsToPopulateStorage()
	{
		ConnectToItemSpawningSystem();
		
		m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!m_SpawningSystem)
			return null;
		
		if (HasItemDataConfig())
		{
			//return m_SpawningSystem.GetItemsFromContainerConfig(m_aItemData, m_Tier, m_Usage, m_Categories, m_iContainerItemMinimum, m_iContainerItemMaximum);
		}
		else
		{
			//return m_SpawningSystem.GetItemsFromUniversalConfig(m_Tier, m_Usage, m_Categories, m_iContainerItemMinimum, m_iContainerItemMaximum);
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when container is searched by user action
	protected void OnContainerSearched(IEntity containerEntity, CE_SearchableContainerComponent container, IEntity searcher)
	{
		array<CE_ItemData> itemsArray = GetItemsToPopulateStorage();
		if (itemsArray && !itemsArray.IsEmpty())
			PopulateStorage(itemsArray);
		
		SetHasBeenSearched(true);
		
		ConnectToTimingSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when container resets
	protected void OnContainerReset(IEntity containerEntity, CE_SearchableContainerComponent container)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(containerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return;
		
		SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(containerEntity.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!ownerStorage)
			return;
		
		array<IEntity> storageItems = {};
		
		storageManager.GetAllItems(storageItems, ownerStorage);
		
		foreach (IEntity storageItem : storageItems)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(storageItem);
		}
		
		Print("Container Has Been Reset");
		
		SetHasContainerReset(false);
		
		SetHasBeenSearched(false);
		
		DisconnectFromItemSpawningSystem();
		DisconnectFromTimingSystem();
		
		SetCurrentContainerResetTime(GetContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Populates searchable container storage with items
	protected void PopulateStorage(array<CE_ItemData> itemsArray)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return;
		
		SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!ownerStorage)
			return;
		
		foreach (CE_ItemData item: itemsArray)
		{
			storageManager.TrySpawnPrefabToStorage(item.m_sPrefab, ownerStorage);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Has the container been searched?
	bool HasBeenSearched()
	{
		return m_bHasBeenSearched;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the container has searched
	void SetHasBeenSearched(bool searched)
	{
		m_bHasBeenSearched = searched;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does this container have a config?
	bool HasItemDataConfig()
	{
		return m_bHasItemDataConfig;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the container has a config
	void SetHasItemDataConfig(bool config)
	{
		m_bHasItemDataConfig = config;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the tier of the container component
	CE_ELootTier GetContainerTier()
	{
		return m_Tier;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the tier of the container component
	void SetContainerTier(CE_ELootTier tier)
	{
		m_Tier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the usage of the container component
	CE_ELootUsage GetContainerUsage()
	{
		return m_Usage;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the usage of the container component
	void SetContainerUsage(CE_ELootUsage usage)
	{
		m_Usage = usage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was the item despawned from container? NOT TAKEN
	bool WereItemsDespawned()
	{
		return m_bWereItemsDespawned;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was despawned from the container
	void SetWereItemsDespawned(bool despawned)
	{
		m_bWereItemsDespawned = despawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the current container reset time
	int GetCurrentContainerResetTime()
	{
		return m_iCurrentContainerResetTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the current container reset time
	void SetCurrentContainerResetTime(int time)
	{
		m_iCurrentContainerResetTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the container reset time
	int GetContainerResetTime()
	{
		return m_iContainerResetTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has the container reset?
	bool HasContainerReset()
	{
		return m_bHasContainerReset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the container has been reset
	void SetHasContainerReset(bool reset)
	{
		m_bHasContainerReset = reset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the container item spawn minimum
	int GetContainerItemMinimum()
	{
		return m_iContainerItemMinimum;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the container item spawn minimum
	int GetContainerItemMaximum()
	{
		return m_iContainerItemMaximum;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets container searched script invoker
	CE_OnContainerSearchedInvoker GetContainerSearchedInvoker()
	{
		return m_OnContainerSearchedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets container reset script invoker
	CE_OnContainerResetInvoker GetContainerResetInvoker()
	{
		return m_OnContainerResetInvoker;
	}
}