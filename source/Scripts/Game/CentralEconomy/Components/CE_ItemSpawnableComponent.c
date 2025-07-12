[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to spawnable items")]
class CE_ItemSpawnableComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawnableComponent : ScriptComponent
{
	protected ref CE_ItemLifetimeEndedInvoker 				m_OnItemLifetimeEndedInvoker 					= new CE_ItemLifetimeEndedInvoker();	// script invoker for when the item's lifetime has ended
	protected ref CE_ItemRestockEndedInvoker 				m_OnItemRestockEndedInvoker 					= new CE_ItemRestockEndedInvoker();	// script invoker for when the item's restock time has ended
	
	protected CE_ItemSpawnableSystem 						m_SpawnableSystem;																// the spawnable item game system
	protected CE_ItemSpawningComponent					m_Spawner;																		// which CE_ItemSpawningComponent does this CE_ItemSpawnableComponent correspond to?
	protected ref CE_Item									m_Item;																			// which CE_Item does this CE_ItemSpawnableComponent correspond to?
	
	protected int										m_iTotalRestockTime							= 0;									// total restock time (set from the CE_ItemDataConfig)
	protected int										m_iCurrentRestockTime							= 0;									// current restock time
	protected int										m_iTotalLifetime								= 0;									// total lifetime (set from the CE_ItemDataConfig)
	protected int										m_iCurrentLifetime							= 0;									// current lifetime
	
	protected bool										m_bHasRestockEnded							= false;								// has this item's restock ended?
	protected bool										m_bHasLifetimeEnded							= false;								// has this item's lifetime ended?
	protected bool										m_bWasItemTaken								= false;								// was item taken and NOT despawned?
	protected bool										m_bWasSpawnedBySystem							= false;								// was item spawned by CE_ItemSpawningSystem?
	
	//------------------------------------------------------------------------------------------------
	//! Post initialization
	protected override void OnPostInit(IEntity owner)
	{
		HookEvents();
		
		GetGame().GetCallqueue().CallLater(DelayedInit, 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delayed initialization call after component creation
	protected void DelayedInit()
	{
		if (WasSpawnedBySystem() && !HasRestockEnded())
			ConnectToItemSpawnableSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize necessary callbacks
	protected void HookEvents()
	{
		m_OnItemLifetimeEndedInvoker.Insert(OnLifetimeEnded);
		m_OnItemRestockEndedInvoker.Insert(OnRestockEnded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called from the m_SpawnableSystem to handle spawnable item lifetime and restock
	void Update(int checkInterval)
	{
		if (GetTotalLifetime() && GetTotalLifetime() != 0)
		{
			SetCurrentLifetime(Math.ClampInt(GetCurrentLifetime() - checkInterval, 0, GetTotalLifetime()));
			if (GetCurrentLifetime() == 0 && !HasLifetimeEnded() && !WasItemTaken())
			{
				SetHasLifetimeEnded(true);
				
				m_OnItemLifetimeEndedInvoker.Invoke(this, GetItem());
			}
			
			//Print("Lifetime: " + GetCurrentLifetime());
		}
		
		if (GetTotalRestockTime() && GetTotalRestockTime() != 0 && WasItemTaken())
		{ 
			SetCurrentRestockTime(Math.ClampInt(GetCurrentRestockTime() - checkInterval, 0, GetTotalRestockTime()));
			if (GetCurrentRestockTime() == 0 && !HasRestockEnded())
			{
				SetHasRestockEnded(true);
				
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
			
			GetSpawner().GetSpawnerResetnvoker().Invoke(GetSpawner());
			
			DisconnectFromItemSpawnableSystem();
			
			SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
		}
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
	//! Gets the CE_ItemSpawningComponent corresponding to this CE_ItemSpawnableComponent
	CE_ItemSpawningComponent GetSpawner()
	{
		return m_Spawner;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemSpawningComponent corresponding to this CE_ItemSpawnableComponent
	void SetSpawner(CE_ItemSpawningComponent spawner)
	{
		m_Spawner = spawner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was item spawned by the CE_ItemSpawningSystem?
	bool WasSpawnedBySystem()
	{
		return m_bWasSpawnedBySystem;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was spawned by the CE_ItemSpawningSystem
	void SetWasSpawnedBySystem(bool spawned)
	{
		m_bWasSpawnedBySystem = spawned;
	}
}