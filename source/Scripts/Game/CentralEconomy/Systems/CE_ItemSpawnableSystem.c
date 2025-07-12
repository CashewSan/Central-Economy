class CE_ItemSpawnableSystem : GameSystem
{
	/* 
		Handles CE_ItemSpawnableComponent timings
	*/
	
	protected ref array<CE_ItemSpawnableComponent> 		m_aSpawnableComponents 		= new array<CE_ItemSpawnableComponent>; 	// array of CE_ItemSpawnableComponents registered to this system
	
	protected float 										m_fTimer						= 0;										// timer for check frequency
	protected float										m_fCheckFrequency			= 1; 									// how often (in seconds) the system will update CE_ItemSpawnableComponents
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, handles timing for the spawnables
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;
		
		if (m_fTimer < m_fCheckFrequency)
			return;
		
		m_fTimer = 0;
		
		// Loop backwards to avoid index issues if the array is modified during iteration
		for (int i = m_aSpawnableComponents.Count() - 1; i >= 0; i--)
		{
			CE_ItemSpawnableComponent spawnableComp = m_aSpawnableComponents[i];
			
			if (!spawnableComp)
				continue;
			
			spawnableComp.Update(m_fCheckFrequency);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Registers spawnable components
	void Register(notnull CE_ItemSpawnableComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aSpawnableComponents.Contains(component))
			m_aSpawnableComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters spawnable components
	void Unregister(notnull CE_ItemSpawnableComponent component)
	{
		m_aSpawnableComponents.RemoveItem(component);
		
		if (m_aSpawnableComponents.IsEmpty())
			Enable(false);
	}
}

void CE_ItemLifetimeEnded(CE_ItemSpawnableComponent itemSpawnable, CE_Item item);
typedef func CE_ItemLifetimeEnded;
typedef ScriptInvokerBase<CE_ItemLifetimeEnded> CE_ItemLifetimeEndedInvoker;

void CE_ItemRestockEnded(CE_ItemSpawnableComponent itemSpawnable, CE_Item item);
typedef func CE_ItemRestockEnded;
typedef ScriptInvokerBase<CE_ItemRestockEnded> CE_ItemRestockEndedInvoker;