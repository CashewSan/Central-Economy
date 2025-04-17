[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to spawnable items (mostly for tracking UID)")]
class CE_ItemSpawnableComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawnableComponent : ScriptComponent
{
	protected CE_ItemSpawnableSystem 						m_SpawnableSystem;														// the spawnable item game system
	protected CE_ELootCategory							m_ItemDataCategory;														// CE_ItemData category that corresponds to this component's parent entity
	protected string										m_ItemDataName;															// CE_ItemData name that corresponds to this component's parent entity
	
	protected string 									m_sItemUID;																// the unique identifier for this component's parent entity
	
	protected int										m_iRestockTime								= 0;							// total restock time
	protected int										m_iCurrentRestockTime							= 0;							// current restock time
	protected int										m_iLifetime									= 0;							// total lifetime
	protected int										m_iCurrentLifetime							= 0;							// current lifetime
	
	protected bool										m_bHasRestockEnded							= false;						// has restock ended?
	protected bool										m_bHasLifetimeEnded							= false;						// has lifetime ended?
	protected bool										m_bWasItemTaken								= false;						// was item taken and NOT despawned?
	protected bool										m_bWasSpawnedBySystem							= false;						// was item spawned by CE Item Spawning system?
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Constructor method, calls ConnectToItemSpawnableSystem()
	protected void CE_ItemSpawnableComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//ConnectToItemSpawnableSystem();
		
		GetGame().GetCallqueue().CallLater(DelayedInit, 1000);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Calls ConnectToItemSpawnableSystem()
	protected override void OnPostInit(IEntity owner)
	{
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
	//! Tick method
	void Update(int checkInterval)
	{
		if (GetLifetime() && GetLifetime() != 0)
		{
			SetCurrentLifetime(Math.ClampInt(GetCurrentLifetime() - checkInterval, 0, GetLifetime()));
			if (GetCurrentLifetime() == 0 && !HasLifetimeEnded() && !WasItemTaken())
			{
				SetHasLifetimeEnded(true);
				
				OnLifetimeEnded();
			}
			
			Print("Lifetime: " + GetCurrentLifetime());
		}
		
		if (GetRestockTime() && GetRestockTime() != 0 && WasItemTaken())
		{ 
			SetCurrentRestockTime(Math.ClampInt(GetCurrentRestockTime() - checkInterval, 0, GetRestockTime()));
			if (GetCurrentRestockTime() == 0 && !HasRestockEnded())
			{
				SetHasRestockEnded(true);
				
				OnRestockEnded();
			}
			
			Print("Restock: " + GetCurrentRestockTime());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawnable system and registers this component
	protected void ConnectToItemSpawnableSystem()
	{
		m_SpawnableSystem = CE_ItemSpawnableSystem.GetInstance();
		if (!m_SpawnableSystem)
			return;
		
		m_SpawnableSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this component
	protected void DisconnectFromItemSpawnableSystem()
	{
		m_SpawnableSystem = CE_ItemSpawnableSystem.GetInstance();
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
	protected void OnRestockEnded()
	{
		if (HasRestockEnded())
		{
			CE_ItemSpawningSystem m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
			if (m_SpawningSystem)
			{
				CE_ItemData item = m_SpawningSystem.GetItemsNotRestockReady().GetKeyByValue(GetItemDataName());
				if (item)
					m_SpawningSystem.GetItemsNotRestockReady().Remove(item);
			}
			
			DisconnectFromItemSpawnableSystem();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when lifetime timer has ended
	protected void OnLifetimeEnded()
	{
		if (HasLifetimeEnded() && !WasItemTaken())
		{
			Print("Disconnecting");
			
			DisconnectFromItemSpawnableSystem();
		}
	}
	
	// Getters/Setters
	//------------------------------------------------------------------------------------------------
	//! Gets the item UID of the spawnable component
	string GetItemUID()
	{
		return m_sItemUID;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the item UID of the spawnable component
	void SetItemUID(string uid)
	{
		m_sItemUID = uid;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the total restock time
	int GetRestockTime()
	{
		return m_iRestockTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the total restock time
	void SetRestockTime(int time)
	{
		m_iRestockTime = time;
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
	int GetLifetime()
	{
		return m_iLifetime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the total lifetime
	void SetLifetime(int time)
	{
		m_iLifetime = time;
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
	//! Sets if the item was taken and NOT despawned
	void SetWasItemTaken(bool taken)
	{
		m_bWasItemTaken = taken;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets corresponding Central Economy item data name
	string GetItemDataName()
	{
		return m_ItemDataName;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets corresponding Central Economy item data name
	void SetItemDataName(string itemDataName)
	{
		m_ItemDataName = itemDataName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets corresponding Central Economy item data category
	CE_ELootCategory GetItemDataCategory()
	{
		return m_ItemDataCategory;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets corresponding Central Economy item data category
	void SetItemDataCategory(CE_ELootCategory category)
	{
		m_ItemDataCategory = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was item spawned by the CE Item Spawning System?
	bool WasSpawnedBySystem()
	{
		return m_bWasSpawnedBySystem;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets if the item was spawned by the CE Item Spawning System
	void SetWasSpawnedBySystem(bool spawned)
	{
		m_bWasSpawnedBySystem = spawned;
	}
}