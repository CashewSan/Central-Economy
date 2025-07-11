modded class SCR_OpenStorageAction : SCR_InventoryAction
{
	protected SCR_ItemAttributeCollection m_StorageItemAttributes;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;
		
		// If target has searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (targetSearchableContainer)
		{
			// If target does not have a loot category
			if (!targetSearchableContainer.m_Categories)
				return false;
			
			// If target has already been searched
			if (!targetSearchableContainer.HasBeenSearched())
				return false;
			
			// If target is destroyed
			SCR_DamageManagerComponent targetDamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
			if (targetDamageManager)
			{
				if (targetDamageManager.IsDestroyed())
					return false;
			}
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
		if (targetSearchableContainer)
		{
			// If target does not have a loot category
			if (!targetSearchableContainer.m_Categories)
				return false;
			
			// If target has already been searched
			if (!targetSearchableContainer.HasBeenSearched())
				return false;
			
			// If target is destroyed
			SCR_DamageManagerComponent targetDamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
			if (targetDamageManager)
			{
				if (targetDamageManager.IsDestroyed())
					return false;
			}
		}
		
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		// If target has searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
		if (targetSearchableContainer)
		{
			SCR_UniversalInventoryStorageComponent ownerStorage = SCR_UniversalInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(SCR_UniversalInventoryStorageComponent));
			if (!ownerStorage)
				return;
			
			m_StorageItemAttributes = SCR_ItemAttributeCollection.Cast(ownerStorage.GetAttributes());
			
			if (m_StorageItemAttributes)
				m_StorageItemAttributes.CE_SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		// If target has searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(owner.FindComponent(CE_SearchableContainerComponent));
		if (targetSearchableContainer)
		{
			if (m_StorageItemAttributes)
				m_StorageItemAttributes.CE_SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
		
		// If target has searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
		if (targetSearchableContainer)
		{
			if (m_StorageItemAttributes)
				m_StorageItemAttributes.CE_SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		// If target has searchable container component
		CE_SearchableContainerComponent targetSearchableContainer = CE_SearchableContainerComponent.Cast(pOwnerEntity.FindComponent(CE_SearchableContainerComponent));
		if (targetSearchableContainer)
		{
			if (m_StorageItemAttributes)
				m_StorageItemAttributes.CE_SetVisible(false);
		}
	}
};