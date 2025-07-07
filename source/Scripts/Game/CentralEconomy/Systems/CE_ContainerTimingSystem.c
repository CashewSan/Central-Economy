class CE_ContainerTimingSystem : GameSystem
{
	protected ref array<CE_SearchableContainerComponent> 	m_aContainerComponents 		= {}; 			// container components registered
	
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
		for (int i = m_aContainerComponents.Count() - 1; i >= 0; i--)
		{
			CE_SearchableContainerComponent comp = m_aContainerComponents[i];
			
			if (!comp)
				continue;
			
			comp.Update(m_fCheckInterval);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Registers searchable container component
	void RegisterContainer(notnull CE_SearchableContainerComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aContainerComponents.Contains(component))
			m_aContainerComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters searchable container component
	void UnregisterContainer(notnull CE_SearchableContainerComponent component)
	{
		m_aContainerComponents.RemoveItem(component);
		
		if (m_aContainerComponents.IsEmpty())
			Enable(false);
	}
}