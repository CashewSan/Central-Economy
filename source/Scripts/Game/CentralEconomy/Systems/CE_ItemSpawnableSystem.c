class CE_ItemSpawnableSystem : GameSystem
{
	protected ref array<CE_ItemSpawnableComponent> 		m_aComponents 				= new array<CE_ItemSpawnableComponent>; 			// initial pull of ALL item spawnable components in the world
	//protected CE_WorldValidationComponent 					m_WorldValidationComponent;													// component added to world's gamemode for verification
	
	protected float 										m_fTimer						= 0;												// timer for spawning check interval
	protected float										m_fCheckInterval				= 0; 											// how often the item spawning system will run (in seconds)
	
	//protected bool 										m_bWorldProcessed			= false;											// has the world been processed?
	
	//------------------------------------------------------------------------------------------------
	//! Sets m_fCheckInterval
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
		
		m_fCheckInterval = 1;
		
		//DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;
		
		if (m_fTimer >= m_fCheckInterval)
		{
			m_fTimer = 0;
			
			foreach (CE_ItemSpawnableComponent comp : m_aComponents)
			{
				if (comp.GetLifetime() != 0)
				{
					comp.SetCurrentLifetime(Math.ClampInt(comp.GetCurrentLifetime() - m_fCheckInterval, 0, comp.GetLifetime()));
					if (comp.GetCurrentLifetime() == 0 && !comp.HasLifetimeEnded() && !comp.WasItemTaken())
					{
						comp.SetHasLifetimeEnded(true);
						
						comp.OnLifetimeEnded();
					}
					
					//Print("Lifetime: " + comp.GetCurrentLifetime());
				}
				
				if (comp.GetRestockTime() != 0)
				{
					comp.SetCurrentRestockTime(Math.ClampInt(comp.GetCurrentRestockTime() - m_fCheckInterval, 0, comp.GetRestockTime()));
					if (comp.GetCurrentRestockTime() == 0 && !comp.HasRestockEnded())
					{
						comp.SetHasRestockEnded(true);
						
						comp.OnRestockEnded();
					}
					
					//Print("Restock: " + comp.GetCurrentRestockTime());
				}
			}
			
			/*
			if (m_bWorldProcessed)
			{
				
			}
			
			else
				GetGame().GetCallqueue().CallLater(DelayedInit, 100, false);
			*/
		}
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Delayed initialization, sets the m_WorldValidationComponent, sets m_WorldProcessed as true if found, and starts the process of spawning
	protected void DelayedInit()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed())
			{	
				m_bWorldProcessed = true;
			}
		}
	}
	*/
	
	// GameSystem stuff
	
	//------------------------------------------------------------------------------------------------
	//! Gets the system instance
	static CE_ItemSpawnableSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawnableSystem.Cast(world.FindSystem(CE_ItemSpawnableSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! Registers component
	void Register(notnull CE_ItemSpawnableComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters component
	void Unregister(notnull CE_ItemSpawnableComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		Enable(false);
	}
}