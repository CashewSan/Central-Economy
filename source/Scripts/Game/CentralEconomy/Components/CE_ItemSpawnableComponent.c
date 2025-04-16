[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to spawnable items (mostly for tracking UID)")]
class CE_ItemSpawnableComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawnableComponent : ScriptComponent
{
	protected CE_ItemSpawnableSystem 						m_SpawnableSystem;														// the spawnable item game system
	protected CE_ItemData									m_ItemData;																// CE_ItemData that corresponds to this component's parent entity
	protected CE_ItemSpawningComponent					m_SpawningComponent;														// CE_ItemSpawningComponent that corresponds to this component's parent entity
	
	protected string 									m_sItemUID;																// the unique identifier for this component's parent entity
	
	protected int										m_iRestockTime								= 0;							// total restock time
	protected int										m_iCurrentRestockTime							= 0;							// current restock time
	protected int										m_iLifetime									= 0;							// total lifetime
	protected int										m_iCurrentLifetime							= 0;							// current lifetime
	
	protected bool										m_bHasRestockEnded							= false;						// has restock ended?
	protected bool										m_bHasLifetimeEnded							= false;						// has lifetime ended?
	protected bool										m_bWasItemTaken								= false;						// was item taken and NOT despawned?
	
	//------------------------------------------------------------------------------------------------
	//! Constructor method, calls ConnectToItemSpawnableSystem()
	protected void CE_ItemSpawnableComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		ConnectToItemSpawnableSystem();
		
		//GetGame().GetCallqueue().CallLater(ConnectToItemSpawnableSystem, 1000);
	}
	
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
			
			//Print("Lifetime: " + comp.GetCurrentLifetime());
		}
		
		if (GetRestockTime() && GetRestockTime() != 0)
		{
			SetCurrentRestockTime(Math.ClampInt(GetCurrentRestockTime() - checkInterval, 0, GetRestockTime()));
			if (GetCurrentRestockTime() == 0 && !HasRestockEnded())
			{
				SetHasRestockEnded(true);
				
				OnRestockEnded();
			}
			
			//Print("Restock: " + comp.GetCurrentRestockTime());
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
	//! Deconstructor method, calls DisconnectFromItemSpawnableSystem()
	protected void ~CE_ItemSpawnableComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		DisconnectFromItemSpawnableSystem();
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
	void OnRestockEnded()
	{
		if (HasRestockEnded())
		{
			CE_ItemSpawningSystem m_SpawningSystem = CE_ItemSpawningSystem.GetInstance();
			if (m_SpawningSystem)
				m_SpawningSystem.GetItemsNotRestockReady().Remove(GetItemData());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when lifetime timer has ended
	void OnLifetimeEnded()
	{
		if (HasLifetimeEnded() && !WasItemTaken())
		{
			GetSpawningComponent().SetWasItemDespawned(true);
			
			SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
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
	//! Gets corresponding Central Economy item data
	CE_ItemData GetItemData()
	{
		return m_ItemData;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets corresponding Central Economy item data
	void SetItemData(CE_ItemData itemData)
	{
		m_ItemData = itemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets corresponding Central Economy spawning component
	CE_ItemSpawningComponent GetSpawningComponent()
	{
		return m_SpawningComponent;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets corresponding Central Economy spawning component
	void SetSpawningComponent(CE_ItemSpawningComponent spawner)
	{
		m_SpawningComponent = spawner;
	}
}