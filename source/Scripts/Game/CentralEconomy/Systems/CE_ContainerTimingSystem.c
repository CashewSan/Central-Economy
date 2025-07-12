class CE_ContainerTimingSystem : GameSystem
{
	/* 
		Handles CE_SearchableContainerComponent timings
	*/
	
	protected ref array<CE_SearchableContainerComponent> 	m_aContainerComponents 		= new array<CE_SearchableContainerComponent>; 			// array of CE_SearchableContainerComponents registered to this system
	
	protected float 										m_fTimer						= 0;													// timer for check frequency
	protected const float									m_fCheckInterval				= 1; 												// how often (in seconds) the system will update CE_SearchableContainerComponents
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, handles updates for CE_SearchableContainerComponents
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