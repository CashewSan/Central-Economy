class CE_ItemSpawnableSystem : GameSystem
{
	protected ref array<CE_ItemSpawnableComponent> 		m_aComponents 				= {}; 			// initial pull of ALL item spawnable components in the world
	
	protected float 										m_fTimer						= 0;				// timer for spawning check interval
	protected float										m_fCheckInterval				= 1; 			// how often the item spawning system will run (in seconds)
	
	//------------------------------------------------------------------------------------------------
	//! Sets m_fCheckInterval
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;
		
		if (m_fTimer < m_fCheckInterval)
			return;
		
		m_fTimer = 0;
		
		foreach (CE_ItemSpawnableComponent comp : m_aComponents)
		{
			comp.Update(m_fCheckInterval);
		}
	}
	
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
		if (!component)
			return;
		
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
		
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
}