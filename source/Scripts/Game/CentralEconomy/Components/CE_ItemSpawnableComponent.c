[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to spawnable items")]
class CE_ItemSpawnableComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawnableComponent : ScriptComponent
{
	protected ref CE_ItemLifetimeEndedInvoker 				m_OnItemLifetimeEndedInvoker 					= new CE_ItemLifetimeEndedInvoker();	// script invoker for when the item's lifetime has ended
	protected ref CE_ItemRestockEndedInvoker 				m_OnItemRestockEndedInvoker 					= new CE_ItemRestockEndedInvoker();	// script invoker for when the item's restock time has ended
	protected ref CE_OnItemDepositedInvoker				m_OnItemDepositedInvoker						= new CE_OnItemDepositedInvoker();	// script invoker for when the item has be deposited into a CE Searchable Container
	protected ref CE_OnItemSpawnedInvoker					m_OnItemSpawnedInvoker						= new CE_OnItemSpawnedInvoker();	// script invoker for when the item has been spawned on the spawner
	
	protected CE_ItemSpawnableSystem 						m_SpawnableSystem;																// the spawnable item game system
	protected CE_Spawner									m_Spawner;																		// which CE_ItemSpawningComponent does this CE_ItemSpawnableComponent correspond to?
	protected ref CE_Item									m_Item;																			// which CE_Item does this CE_ItemSpawnableComponent correspond to?
	
	protected int										m_iTotalRestockTime							= 0;									// total restock time (set from the CE_ItemDataConfig)
	protected int										m_iCurrentRestockTime							= 0;									// current restock time
	protected int										m_iTotalLifetime								= 0;									// total lifetime (set from the CE_ItemDataConfig)
	protected int										m_iCurrentLifetime							= 0;									// current lifetime
	
	protected bool										m_bHasRestockEnded							= false;								// has this item's restock ended?
	protected bool										m_bHasLifetimeEnded							= false;								// has this item's lifetime ended?
	protected bool										m_bWasItemTaken								= false;								// was item taken from it's spawner or searchable container?
	
	[RplProp()]
	protected bool										m_bWasDepositedByAction						= false;								// was item deposited into CE Searchable Containers by the user action?
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the deposit event callback
	void HookDepositEvent()
	{
		m_OnItemDepositedInvoker.Insert(OnItemDeposited);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize the necessary callbacks for spawning on a spawner
	void HookSpawningEvents()
	{
		m_OnItemSpawnedInvoker.Insert(OnItemSpawned);
		m_OnItemLifetimeEndedInvoker.Insert(OnLifetimeEnded);
		m_OnItemRestockEndedInvoker.Insert(OnRestockEnded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called from the m_SpawnableSystem to handle spawnable item lifetime and restock
	void Update(int checkInterval)
	{
		if (GetTotalLifetime() && GetTotalLifetime() != 0  && !WasItemTaken())
		{
			m_iCurrentLifetime = Math.ClampInt(GetCurrentLifetime() - checkInterval, 0, GetTotalLifetime());
			if (GetCurrentLifetime() == 0 && !HasLifetimeEnded())
			{
				m_bHasLifetimeEnded = true;
				
				m_OnItemLifetimeEndedInvoker.Invoke(this, GetItem());
			}
			
			//Print("Lifetime: " + GetCurrentLifetime());
		}
		
		if (GetTotalRestockTime() && GetTotalRestockTime() != 0 && WasItemTaken())
		{ 
			m_iCurrentRestockTime = Math.ClampInt(GetCurrentRestockTime() - checkInterval, 0, GetTotalRestockTime());
			if (GetCurrentRestockTime() == 0 && !HasRestockEnded())
			{
				m_bHasRestockEnded = true;
				
				m_OnItemRestockEndedInvoker.Invoke(this, GetItem());
			}
			
			//Print("Restock: " + GetCurrentRestockTime());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawnable system and registers this component
	protected void ConnectToItemSpawnableSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawnableSystem = CE_ItemSpawnableSystem.Cast(world.FindSystem(CE_ItemSpawnableSystem));
		if (!m_SpawnableSystem)
			return;
		
		m_SpawnableSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromItemSpawnableSystem()
	{
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawnableSystem = CE_ItemSpawnableSystem.Cast(world.FindSystem(CE_ItemSpawnableSystem));
		if (!m_SpawnableSystem)
			return;

		m_SpawnableSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item is spawned onto a CE_Spawner
	protected void OnItemSpawned(CE_Item item, CE_Spawner spawner)
	{
		m_Item = item;
		m_Spawner = spawner;
		
		CE_ItemData itemData = item.GetItemData();
		if (itemData)
		{
			m_iTotalRestockTime = itemData.m_iRestock;
			m_iCurrentRestockTime = itemData.m_iRestock;
			m_iTotalLifetime = itemData.m_iLifetime;
			m_iCurrentLifetime = itemData.m_iLifetime;
		}
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnItemTaken);
		
		ConnectToItemSpawnableSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item is deposited into a CE Searchable Container
	protected void OnItemDeposited(CE_Item item, CE_SearchableContainerComponent container)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnParentSlotChangedInvoker.Insert(OnItemTaken);
		
		CE_ItemData itemData = item.GetItemData();
		if (itemData)
		{
			m_iTotalRestockTime = itemData.m_iRestock;
			m_iCurrentRestockTime = itemData.m_iRestock;
			// won't set lifetime on this because the items will never despawn in a searchable container, only when the container resets will they be despawned
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the item is taken from a CE Searchable Container or a CE_Spawner
	protected void OnItemTaken(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		m_bWasItemTaken = true;
		m_iTotalLifetime = 0;
		
		ConnectToItemSpawnableSystem();
		
		if (WasDepositedByAction())
			m_bWasDepositedByAction = false; // can no longer be put back into searchable container
		
		World world = GetOwner().GetWorld();
		if (world)
		{
			CE_ItemSpawningSystem spawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
			if (spawningSystem)
			{
				spawningSystem.SetItemCount(spawningSystem.GetItemCount() - 1);
			}
		}
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
		{
			itemComponent.m_OnParentSlotChangedInvoker.Remove(OnItemTaken);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls for disconnect from item spawnable system on deletion of component
	override void OnDelete(IEntity owner)
	{
		DisconnectFromItemSpawnableSystem();

		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when restock timer has ended
	protected void OnRestockEnded(CE_ItemSpawnableComponent itemSpawnable, CE_Item item)
	{
		if (HasRestockEnded())
		{
			item.SetAvailableCount(item.GetAvailableCount() + 1);
			
			DisconnectFromItemSpawnableSystem();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when lifetime timer has ended
	protected void OnLifetimeEnded(CE_ItemSpawnableComponent itemSpawnable, CE_Item item)
	{
		if (HasLifetimeEnded() && !WasItemTaken())
		{
			item.SetAvailableCount(item.GetAvailableCount() + 1);
			
			if (GetSpawner())
			{
				CE_ItemSpawningComponent spawnerComp = GetSpawner().GetSpawningComponent();
			
				if (spawnerComp)
					spawnerComp.GetSpawnerResetnvoker().Invoke(spawnerComp);
			}
			
			DisconnectFromItemSpawnableSystem();
			
			SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// REPLICATION STUFF
	//------------------------------------------------------------------------------------------------
	
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.Write(m_bWasDepositedByAction, 1);
		
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(m_bWasDepositedByAction, 1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Gets the total restock time
	int GetTotalRestockTime()
	{
		return m_iTotalRestockTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the total restock time
	void SetTotalRestockTime(int time)
	{
		m_iTotalRestockTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the current restock time
	int GetCurrentRestockTime()
	{
		return m_iCurrentRestockTime;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the current restock time
	void SetCurrentRestockTime(int time)
	{
		m_iCurrentRestockTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the total lifetime
	int GetTotalLifetime()
	{
		return m_iTotalLifetime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the total lifetime
	void SetTotalLifetime(int time)
	{
		m_iTotalLifetime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the current lifetime
	int GetCurrentLifetime()
	{
		return m_iCurrentLifetime;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the current lifetime
	void SetCurrentLifetime(int time)
	{
		m_iCurrentLifetime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has restock ended?
	bool HasRestockEnded()
	{
		return m_bHasRestockEnded;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the restock has ended
	void SetHasRestockEnded(bool ended)
	{
		m_bHasRestockEnded = ended;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has restock ended?
	bool HasLifetimeEnded()
	{
		return m_bHasLifetimeEnded;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the restock has ended
	void SetHasLifetimeEnded(bool ended)
	{
		m_bHasLifetimeEnded = ended;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was item taken and NOT despawned?
	bool WasItemTaken()
	{
		return m_bWasItemTaken;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was taken, NOT despawned
	void SetWasItemTaken(bool taken)
	{
		m_bWasItemTaken = taken;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was the item deposited into the CE Searchable Container by the user action?
	bool WasDepositedByAction()
	{
		return m_bWasDepositedByAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was deposited into the CE Searchable Container by the user action
	void SetWasDepositedByAction(bool deposited)
	{
		m_bWasDepositedByAction = deposited;
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the CE_Item corresponding to this CE_ItemSpawnableComponent
	CE_Item GetItem()
	{
		return m_Item;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_Item corresponding to this CE_ItemSpawnableComponent
	void SetItem(CE_Item item)
	{
		m_Item = item;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the CE_Spawner corresponding to this CE_ItemSpawnableComponent
	CE_Spawner GetSpawner()
	{
		return m_Spawner;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_Spawner corresponding to this CE_ItemSpawnableComponent
	void SetSpawner(CE_Spawner spawner)
	{
		m_Spawner = spawner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item deposited invoker
	CE_OnItemDepositedInvoker GetItemDepositedInvoker()
	{
		return m_OnItemDepositedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item deposited invoker
	CE_OnItemSpawnedInvoker GetItemSpawnedInvoker()
	{
		return m_OnItemSpawnedInvoker;
	}
}