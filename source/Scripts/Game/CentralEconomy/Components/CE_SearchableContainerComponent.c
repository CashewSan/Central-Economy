[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to searchable containers")]
class CE_SearchableContainerComponentClass : ScriptComponentClass
{
}

class CE_SearchableContainerComponent : ScriptComponent
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used (If set, universal config through CE_WorldValidationComponent will be ignored!)", "conf", category: "Container Data")]
	ref CE_ItemDataConfig m_ItemDataConfig;
	
	[Attribute("", UIWidgets.ComboBox, desc: "Which spawn location(s) will this container spawn items in? (If set, usages set throughout the world will be ignored!)", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Container Data")]
	protected CE_ELootUsage m_ContainerUsage;
	
	[Attribute("", UIWidgets.Flags, desc: "Which category of items do you want to spawn here?", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Container Data")]
	protected CE_ELootCategory m_Categories;
	
	[Attribute("1800", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the container to reset after it was initially searched. Helps prevent loot camping.", params: "10 inf 10", category: "Container Data")] // default set to 1800 seconds (30 minutes)
	int m_iContainerResetTime;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Minimum amount of items you want this searchable container to potentially spawn.", params: "0 inf 1", category: "Container Data")] // default set to 0
	int m_iContainerItemMinimum;
	
	[Attribute("3", UIWidgets.EditBox, desc: "Maximum amount of items you want this searchable container to potentially spawn.", params: "0 inf 1", category: "Container Data")] // default set to 3
	int m_iContainerItemMaximum;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
	
	protected bool 										m_bHasConfig 							= false;								// does the searchable container have a custom item data config? (NOT THE UNIVERSAL ONE)
	
	/*[RplProp()]*/
	protected bool 										m_bHasBeenSearched						= false;								// has the container been searched?
	
	[RplProp()]
	protected bool										m_bIsSearchable							= false;								// is the container searchable?
	
	protected bool 										m_bWereItemsDespawned 					= false;								// was the items despawned by the system?
	protected bool										m_bHaveItemsProcessed	;														// have the items of the container been processed? ONLY APPLICABLE IF m_ItemDataConfig IS SET!
	protected bool										m_bHasContainerReset						= false;								// has the container reset?
	protected bool										m_bHasUsage								= false;								// does the container have a usage set through the component?
	protected bool 										m_bReadyForItems 						= true;								// has item spawned on the spawner?
	
	protected int										m_iCurrentContainerResetTime				= 0;									// current container reset time
	
	protected ref CE_OnContainerSearchedInvoker 			m_OnContainerSearchedInvoker 				= new CE_OnContainerSearchedInvoker();	// script invoker for when the container is searched
	protected ref CE_OnContainerResetInvoker 				m_OnContainerResetInvoker 				= new CE_OnContainerResetInvoker();	// script invoker for when the container is reset
	protected ref CE_OnAreasQueriedInvoker					m_OnAreasQueriedInvoker					= new CE_OnAreasQueriedInvoker();		// script invoker for when all areas have been queried
	
	protected ref array<ref CE_Item>						m_aItems									= new array<ref CE_Item>;				// CE_Item array, items processed from system IF the container has it's own config set (m_ItemDataConfig)
	protected ref array<ref CE_Item> 						m_aItemsSpawned							= new array<ref CE_Item>;				// CE_Item array that has spawned on the container
	protected ref array<IEntity> 							m_EntitiesSpawned						= new array<IEntity>;					// IEntity array that has spawned on the container
	
	//[RplProp()]
	//protected ref CE_SearchableContainer					m_Container								/*= new CE_SearchableContainer()*/;		// CE_SearchableContainer corresponding to this CE_SearchableContainerComponent
	protected ref RandomGenerator 						m_RandomGen								= new RandomGenerator();				// vanilla random generator
	
	protected CE_ItemSpawningSystem 						m_SpawningSystem;															// the spawning game system used to control spawning
	protected CE_ContainerTimingSystem					m_TimingSystem;																// the timing system for controlling container reset
	
	//------------------------------------------------------------------------------------------------
	//! Event on post initiailization
	protected override void OnPostInit(IEntity owner)
	{
		owner.ClearFlags(EntityFlags.STATIC);
		
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.GetOnAreasQueriedInvoker().Insert(OnAreasQueried);
	}
	
	protected void OnAreasQueried()
	{
		CE_WorldValidationComponent m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		if (!m_WorldValidationComponent)
			return;
		
		float randomFloat = m_RandomGen.RandFloat01();
		
		if (m_WorldValidationComponent.GetSearchableContainerChance() < randomFloat)
			return;
		
		SetIsSearchable(true);
		
		//Replication.BumpMe();
		
		HookEvents();
		
		if (m_ContainerUsage)
		{
			m_bHasUsage = true;
			SetContainerUsage(m_ContainerUsage);
		}
		
		if (m_ItemDataConfig)
			LoadConfig();
		else
			ConnectToItemSpawningSystem();
		
		SetCurrentContainerResetTime(GetContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method
	void Update(int checkInterval)
	{
		SetCurrentContainerResetTime(Math.ClampInt(GetCurrentContainerResetTime() - checkInterval, 0, GetContainerResetTime()));
		if (GetCurrentContainerResetTime() == 0 && HasBeenSearched())
		{
			m_OnContainerResetInvoker.Invoke(this);
		}
		
		//Print("Timer: " + GetCurrentContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize necessary callbacks
	protected void HookEvents()
	{
		m_OnContainerSearchedInvoker.Insert(OnContainerSearched);
		m_OnContainerResetInvoker.Insert(OnContainerReset);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this component
	protected void ConnectToItemSpawningSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterContainer(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromItemSpawningSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterContainer(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this component
	protected void ConnectToTimingSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_TimingSystem = CE_ContainerTimingSystem.Cast(world.FindSystem(CE_ContainerTimingSystem));
		if (!m_TimingSystem)
			return;
		
		m_TimingSystem.RegisterContainer(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromTimingSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_TimingSystem = CE_ContainerTimingSystem.Cast(world.FindSystem(CE_ContainerTimingSystem));
		if (!m_TimingSystem)
			return;

		m_TimingSystem.UnregisterContainer(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads CE_ItemData config containing items, deciding if it's one attached to the component or the universal config
	protected void LoadConfig()
	{
		if (m_ItemDataConfig)
		{
			SetHasConfig(true);
			
			World world = GetOwner().GetWorld();
			if (!world)
				return;
			
			m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
			if (!m_SpawningSystem)
				return;
			
			foreach (CE_Item item : m_SpawningSystem.ProcessItemData(m_ItemDataConfig))
			{
				m_aItems.Insert(item);
			}
			
			SetHaveItemsProcessed(false);
		}
		else
			Print("[CentralEconomy::CE_SearchableContainerComponent] NO ITEM DATA CONFIG FOUND!", LogLevel.ERROR); // this should realistically never happen
		
		ConnectToItemSpawningSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when container is searched by user action
	protected void OnContainerSearched(CE_SearchableContainerComponent container, IEntity userEntity)
	{	
		Rpc(RpcAsk_SetHasBeenSearched, true);
		
		ConnectToTimingSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when container resets
	protected void OnContainerReset(CE_SearchableContainerComponent container)
	{
		IEntity containerEntity = container.GetOwner();
		if (containerEntity)
			return;
		
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
		
		SetHasContainerReset(false);
		
		SetHasBeenSearched(false);
		
		DisconnectFromItemSpawningSystem();
		DisconnectFromTimingSystem();
		
		SetCurrentContainerResetTime(GetContainerResetTime());
	}
	
	//------------------------------------------------------------------------------------------------
	// REPLICATION STUFF
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! 
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetHasBeenSearched(bool searched)
	{
		Print("we are server");
		
		m_bHasBeenSearched = searched;
		
		Rpc(RpcDo_SetHasBeenSearched, searched);
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetHasBeenSearched(bool searched)
	{
		Print("we are broadcast");
		
		m_bHasBeenSearched = searched;
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Gets CE_SearchableContainer corresponding to this component
	CE_SearchableContainer GetContainer()
	{
		return m_Container;
	}
	
	
	[RplProp()]
	bool m_bTest = false;
	
	//------------------------------------------------------------------------------------------------
	//! Sets CE_SearchableContainer corresponding to this component
	void SetContainer(CE_SearchableContainer container)
	{
		m_Container = container;
		
		m_bTest = true;
		
		Replication.BumpMe();
		
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		Print("RplRole: " + systemRplNode.GetRole());
		
		if (systemRplNode.GetRole() == RplRole.Authority)
		{
		}
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Is the spawner ready for an item to be spawned on it?
	bool IsReadyForItems()
	{
		return m_bReadyForItems;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the spawner is ready for an item to be spawned on it
	void SetReadyForItems(bool ready)
	{
		m_bReadyForItems = ready;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is container searchable?
	bool IsSearchable()
	{
		return m_bIsSearchable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if container is searchable
	void SetIsSearchable(bool searchable)
	{
		m_bIsSearchable = searchable;
	}
	
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
	//! Does this container have a config set by the component?
	bool HasConfig()
	{
		return m_bHasConfig;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the container has a config set by the component
	void SetHasConfig(bool config)
	{
		m_bHasConfig = config;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does this container have a usage set by the component?
	bool HasUsage()
	{
		return m_bHasUsage;
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
	//! Gets the categories of the container component
	CE_ELootCategory GetContainerCategories()
	{
		return m_Categories;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the categories of the container component
	void SetContainerCategories(CE_ELootCategory category)
	{
		m_Categories = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Have items of this container been processed (only applies if container has it's own config set, will return null otherwise)
	bool HaveItemsProcessed()
	{
		if (HasConfig())
			return m_bHaveItemsProcessed;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if items of this container have been processed (only applies if container has it's own config set)
	void SetHaveItemsProcessed(bool processed)
	{
		m_bHaveItemsProcessed = processed;
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
	//! Gets the container's item data config, if set (if NOT set, will return null)
	array<ref CE_Item> GetItems()
	{
		if (m_aItems.IsEmpty())
			return null;
		
		return m_aItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets CE_Item spawned on the spawning component
	array<ref CE_Item> GetItemsSpawned()
	{
		return m_aItemsSpawned;
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