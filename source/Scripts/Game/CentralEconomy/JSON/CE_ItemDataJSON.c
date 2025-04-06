/*
class CE_ItemDataNEW : Managed
{
	string m_sName;
	ResourceName m_sPrefab;
	vector m_vItemRotation;
	
	int m_iNominal;
	int m_iMinimum;
	int m_iLifetime;
	int m_iRestock;
	int m_iQuantityMaximum;
	int m_iQuantityMinimum;
	
	CE_ELootCategory m_ItemCategory;
	ref array<CE_ELootUsage> m_ItemUsages = {};
	ref array<CE_ELootTier> m_ItemTiers = {};
	
	string m_sItemCategory;
	ref array<string> m_sItemUsages;
	ref array<string> m_sItemTiers;
	
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!context.IsValid())
			return false;
		
		context.WriteValue("Name", m_sName);
		context.WriteValue("Prefab", m_sPrefab);
		context.WriteValue("Rotation", m_vItemRotation);
		
		context.WriteValue("Nominal", m_iNominal);
		context.WriteValue("Minimum", m_iMinimum);
		context.WriteValue("Lifetime", m_iLifetime);
		context.WriteValue("Restock", m_iRestock);
		context.WriteValue("QuantityMax", m_iQuantityMaximum);
		context.WriteValue("QuantityMin", m_iQuantityMinimum);
		
		context.WriteValue("Category", m_sItemCategory);
		context.WriteValue("Usages", m_sItemUsages);
		context.WriteValue("Tiers", m_sItemTiers);
		
		return true;
	
	}
	
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!context.IsValid())
			return false;
		
		context.ReadValue("Name", m_sName);
		context.ReadValue("Prefab", m_sPrefab);
		context.ReadValue("Rotation", m_vItemRotation);
		
		context.ReadValue("Nominal", m_iNominal);
		context.ReadValue("Minimum", m_iMinimum);
		context.ReadValue("Lifetime", m_iLifetime);
		context.ReadValue("Restock", m_iRestock);
		context.ReadValue("QuantityMax", m_iQuantityMaximum);
		context.ReadValue("QuantityMin", m_iQuantityMinimum);
		
		context.ReadValue("Category", m_sItemCategory);
		context.ReadValue("Usages", m_sItemUsages);
		context.ReadValue("Tiers", m_sItemTiers);
		
		return true;
	}
}

class CE_ItemDataJson : JsonApiStruct
{
	ref array<ref CE_ItemDataNEW> m_ItemData = {};
	
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!context.IsValid())
			return false;
		
		context.WriteValue("ItemData", m_ItemData);
		
		return true;
	}
	
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!context.IsValid())
			return false;
		
		context.ReadValue("ItemData", m_ItemData);
		
		return true;
	}
}