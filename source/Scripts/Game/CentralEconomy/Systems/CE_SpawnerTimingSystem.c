class CE_SpawnerTimingSystem : GameSystem
{
	/* 
		Handles CE_ItemSpawningComponent timings
	*/
	
	protected ref array<CE_ItemSpawningComponent> 			m_aSpawnerComponents 			= new array<CE_ItemSpawningComponent>; 			// array of CE_ItemSpawningComponents registered to this system
	
	protected float 										m_fTimer						= 0;												// timer for check frequency
	protected const float									m_fCheckInterval				= 1; 											// how often (in seconds) the system will update CE_ItemSpawningComponents
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, handles updates for CE_ItemSpawningComponents
	override event protected void OnUpdate(ESystemPoint point)
	{
		const RplId systemRplId = Replication.FindItemId(this);
		const RplNode systemRplNode = Replication.FindNode(systemRplId);
		
		if (systemRplNode && systemRplNode.GetRole() == RplRole.Proxy)
			return;
		
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
	//! Returns instance of system within the entity's world
	static CE_SpawnerTimingSystem GetByEntityWorld(IEntity entity)
	{
		World world = entity.GetWorld();
		if (!world)
			return null;

		return CE_SpawnerTimingSystem.Cast(world.FindSystem(CE_SpawnerTimingSystem));
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