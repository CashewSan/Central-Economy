class CE_SearchContainerUserAction : SCR_InventoryAction
{	
	[Attribute(ResourceName.Empty, UIWidgets.Auto, "Audio to be used when searching the container")]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfig;
	
	protected SCR_ItemAttributeCollection m_StorageItemAttributes;
	protected UIInfo m_StorageUIInfo;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		// If target does not have searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (!targetSearchableContainer)
			return false;
		
		// If target does not have a loot category
		if (!targetSearchableContainer.m_Categories)
			return false;
		
		// If target has already been searched
		if (targetSearchableContainer.HasBeenSearched())
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
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		// If target does not have searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (!targetSearchableContainer)
			return false;
		
		// If target does not have a loot category
		if (!targetSearchableContainer.m_Categories)
			return false;
		
		// If target has already been searched
		if (targetSearchableContainer.HasBeenSearched())
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
		
		if (!pOwnerEntity)
			return;
		
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
		
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
		if (!targetSearchableContainer)
			return;
		
		targetSearchableContainer.GetContainerSearchedInvoker().Invoke(pOwnerEntity, targetSearchableContainer, pUserEntity);
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
		outName = "#CE-UserAction_Search" + " " + SCR_StringHelper.Translate(m_StorageUIInfo.GetName());
		return true;
	}
}