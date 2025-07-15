class CE_SearchContainerUserAction : SCR_InventoryAction
{	
	[Attribute(ResourceName.Empty, UIWidgets.Auto, "Audio to be used when searching the container")]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfig;
	
	protected SCR_ItemAttributeCollection 									m_StorageItemAttributes;											// SCR_ItemAttributeCollection of the searchable container entity
	protected UIInfo 													m_StorageUIInfo;													// UIInfo of the searchable container entity
	
	protected CE_ItemSpawningSystem 										m_SpawningSystem;												// item spawning system that handles all item spawning with CentralEconomy (A.K.A. the brain)
	
	protected CE_SearchableContainerComponent 								m_ContainerComponent;												// CE_SearchableContainerComponent corresponding to the searchable container entity
	
	protected ref CE_SearchableContainer									m_Container;														// CE_SearchableContainer corresponding to the searchable container entity
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		/*
		// If no CE_ItemSpawningSystem
		if (!m_SpawningSystem)
			return false;
		
		// If CE_ItemSpawningSystem has not finished having areas queried
		if (!m_SpawningSystem.HaveAreasQueried())
			return false;
		*/
		
		// If no owner entity
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(owner.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!ownerStorage)
			return false;
		
		m_StorageItemAttributes = SCR_ItemAttributeCollection.Cast(ownerStorage.GetAttributes());
		
		if (m_StorageItemAttributes)
		{
			m_StorageUIInfo = m_StorageItemAttributes.GetUIInfo();
		}
		
		// If target does not have searchable container component
		m_ContainerComponent = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (!m_ContainerComponent)
			return false;
		
		// If the CE_SearchableContainerComponent has not been processed into a CE_SearchableContainer
		m_Container = m_ContainerComponent.GetContainer();
		
		Print(m_Container);
		
		if (!m_Container)
			return false;
		
		// If target has already been searched
		if (m_ContainerComponent.HasBeenSearched())
			return false;
		
		// If target is destroyed
		SCR_DamageManagerComponent targetDamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
		if (targetDamageManager)
		{
			if (targetDamageManager.IsDestroyed())
				return false;
		}
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		/*
		// If no CE_ItemSpawningSystem
		if (!m_SpawningSystem)
			return false;
		
		// If CE_ItemSpawningSystem has not finished having areas queried
		if (!m_SpawningSystem.HaveAreasQueried())
			return false;
		*/
		
		// If no owner entity
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		// If target does not have searchable container component
		m_ContainerComponent = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (!m_ContainerComponent)
			return false;
		
		// If the CE_SearchableContainerComponent has not been processed into a CE_SearchableContainer
		m_Container = m_ContainerComponent.GetContainer();
		if (!m_Container)
			return false;
		
		// If target has already been searched
		if (m_ContainerComponent.HasBeenSearched())
			return false;
		
		// If target is destroyed
		SCR_DamageManagerComponent targetDamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
		if (targetDamageManager)
		{
			if (targetDamageManager.IsDestroyed())
				return false;
		}
		
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		if (GetGame().InPlayMode())
		{
			if (!pOwnerEntity)
				return;
			
			World world = pOwnerEntity.GetWorld();
			if (!world)
				return;
			
			m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
			if (!m_SpawningSystem)
				return;
			
			m_ContainerComponent = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
			if (!m_ContainerComponent)
				return;
			
			m_Container = m_ContainerComponent.GetContainer();
			
			SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(SCR_UniversalInventoryStorageComponent));
			if (!ownerStorage)
				return;
			
			m_StorageItemAttributes = SCR_ItemAttributeCollection.Cast(ownerStorage.GetAttributes());
			
			if (m_StorageItemAttributes)
			{
				m_StorageItemAttributes.CE_SetVisible(false);
				
				m_StorageUIInfo = m_StorageItemAttributes.GetUIInfo();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		PlaySound(owner);
		
		if (m_StorageItemAttributes)
			m_StorageItemAttributes.CE_SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;
		
		StopSound(pOwnerEntity);
		
		if (m_StorageItemAttributes)
			m_StorageItemAttributes.CE_SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{	
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		World world = pOwnerEntity.GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;
		
		m_ContainerComponent = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
		if (!m_ContainerComponent)
			return;
		
		if (m_Container)
		{
			m_ContainerComponent.GetContainerSearchedInvoker().Invoke(m_Container, pUserEntity);
		
			TryToPopulateStorage(m_Container);
		}
	}
	
	protected void TryToPopulateStorage(CE_SearchableContainer container)
	{
		if (!container)
			return;
		
		World world = GetOwner().GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;
		
		Print(container.GetContainerRplId());
		
		CE_SearchableContainerComponent containerComp = container.GetContainerComponentFromRplId(container.GetContainerRplId());
		if (!containerComp)
			return;
		
		array<ref CE_Item> items = {};
		
		if (containerComp.HasConfig() && containerComp.HaveItemsProcessed())
		{
			items = containerComp.GetItems();
		}
		else
		{
			items = m_SpawningSystem.GetItems();
		}
		
		array<ref CE_Item> itemsSelected = m_SpawningSystem.SelectItems(items, container, containerComp.GetContainerItemMinimum(), containerComp.GetContainerItemMaximum());
		
		Print(itemsSelected);
		
		if (itemsSelected && !itemsSelected.IsEmpty())
		{
			Print("meow itemsSelected");
			m_SpawningSystem.TryToSpawnItems(container, itemsSelected);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (manager && pOwnerEntity)
			manager.SetStorageToOpen(pOwnerEntity);
			manager.OpenInventory();
		
		if (m_StorageItemAttributes)
			m_StorageItemAttributes.CE_SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Plays sound on entity
	protected void PlaySound(IEntity owner)
	{
		if (!owner)
			return;
		
		// Get SoundManagerEntity
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity || !m_AudioSourceConfig || !m_AudioSourceConfig.IsValid())
			return;
		
		// Get AudioSource
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(owner, m_AudioSourceConfig);
		if (!audioSource)
			return;
		
		// Set position
		vector mat[4];
		owner.GetTransform(mat);
		
		// Get center of bounding box
		vector mins;
		vector maxs;
		owner.GetWorldBounds(mins, maxs);
		mat[3] = vector.Lerp(mins, maxs, 0.5);
		
		// Play sound
		soundManagerEntity.PlayAudioSource(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stops sound on entity
	protected void StopSound(IEntity owner)
	{
		if (!owner)
			return;
		
		// Get SoundManagerEntity
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		soundManagerEntity.TerminateAudioSource(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adjusts action name script
	override bool GetActionNameScript(out string outName)
	{
		if (m_StorageUIInfo)
		{
			outName = "#CE-UserAction_Search" + " " + SCR_StringHelper.Translate(m_StorageUIInfo.GetName());
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
}