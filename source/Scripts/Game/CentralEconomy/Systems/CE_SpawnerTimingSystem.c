class CE_SpawnerTimingSystem : GameSystem
{
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents 			= {}; 			// spawner components registered
	
	protected float 										m_fTimer						= 0;				// current timer count
	protected float										m_fCheckInterval				= 1; 			// how often the system will check (in seconds)
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, currently set to control timer for spawner resets every n seconds
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		m_fTimer += timeSlice;
		
		if (m_fTimer < m_fCheckInterval)
			return;
		
		m_fTimer = 0;
		
		// Loop backwards to avoid index issues if the array is modified during iteration
		for (int i = m_aSpawnerComponents.Count() - 1; i >= 0; i--)
		{
			CE_ItemSpawningComponent comp = m_aSpawnerComponents[i];
			
			if (!comp)
				continue;
			
			comp.Update(m_fCheckInterval);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers spawner component
	void RegisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aSpawnerComponents.Contains(component))
			m_aSpawnerComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawner component
	void UnregisterSpawner(notnull CE_ItemSpawningComponent component)
	{
		m_aSpawnerComponents.RemoveItem(component);
		
		if (m_aSpawnerComponents.IsEmpty())
			Enable(false);
	}
}