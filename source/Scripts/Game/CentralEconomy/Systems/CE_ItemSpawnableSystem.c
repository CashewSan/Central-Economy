class CE_ItemSpawnableSystem : GameSystem
{
	protected ref array<CE_ItemSpawnableComponent> 		m_aComponents 				= {}; 			// initial pull of ALL item spawnable components in the world
	
	protected float 										m_fTimer						= 0;				// timer for spawning check interval
	protected float										m_fCheckInterval				= 1; 			// how often the item spawning system will run (in seconds)
	
	//------------------------------------------------------------------------------------------------
	//! Tick method
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;
		
		if (m_fTimer < m_fCheckInterval)
			return;
		
		m_fTimer = 0;
		
		// Loop backwards to avoid index issues if the array is modified during iteration
		for (int i = m_aComponents.Count() - 1; i >= 0; i--)
		{
			CE_ItemSpawnableComponent comp = m_aComponents[i];
			
			if (!comp)
				continue;
			
			comp.Update(m_fCheckInterval);
		}
		
		//Print("ItemSpawnableCount: " + m_aComponents.Count());
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