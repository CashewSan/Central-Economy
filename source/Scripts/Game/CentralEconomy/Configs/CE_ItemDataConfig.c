[BaseContainerProps(), BaseContainerCustomTitleField("m_sName")]
class CE_ItemData
{
	[Attribute("", UIWidgets.Auto, desc: "Name of Item **MUST BE UNIQUE**")]
	string m_sName;
	
    [Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Prefab to be spawned", "et")]
    ResourceName m_sPrefab;
	
	[Attribute("0 0 0", UIWidgets.EditBox)]
	vector m_vItemRotation;
	
	[Attribute("2", UIWidgets.EditBox, params: "1 10000", desc: "Ideal amount of spawns of this item around the map")]
	int m_iNominal;
	
	[Attribute("1", UIWidgets.EditBox, params: "1 10000", desc: "Minimum amount of spawns of this item around the map **SHOULD BE THE SAME OR LESS THAN THE NOMINAL VALUE**")]
	int m_iMinimum;
	
	[Attribute("14400", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) item can stay on the ground before despawning")]
	int m_iLifetime;
	
	[Attribute("3600", UIWidgets.EditBox, desc: "Length of time (in seconds) for item to spawn upon removal",  params: "0 3888000")]
	int m_iRestock;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Maximum quantity of item (if applicable)")]
	int m_iQuantityMaximum;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Minimum quantity of item (if applicable)")]
	int m_iQuantityMinimum;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Category of item", enums: ParamEnumArray.FromEnum(CE_ELootCategory))]
	CE_ELootCategory m_ItemCategory;
	
	[Attribute("", UIWidgets.Flags, desc: "Which spawn location(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootUsage))]
	CE_ELootUsage m_ItemUsages;
	
	[Attribute("", UIWidgets.Flags, desc: "Which tier(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootTier))]
	CE_ELootTier m_ItemTiers;
	
	/*
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static bool Extract(CE_ItemData instance, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeString(instance.m_sName);
		snapshot.SerializeString(instance.m_sPrefab);
		snapshot.SerializeVector(instance.m_vItemRotation);
		snapshot.SerializeInt(instance.m_iNominal);
		snapshot.SerializeInt(instance.m_iMinimum);
		snapshot.SerializeInt(instance.m_iLifetime);
		snapshot.SerializeInt(instance.m_iRestock);
		snapshot.SerializeInt(instance.m_iQuantityMinimum);
		snapshot.SerializeInt(instance.m_iQuantityMaximum);
		snapshot.SerializeInt(instance.m_ItemCategory);
		snapshot.SerializeInt(instance.m_ItemUsages);
		snapshot.SerializeInt(instance.m_ItemTiers);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, CE_ItemData instance) 
	{
		snapshot.SerializeString(instance.m_sName);
		snapshot.SerializeString(instance.m_sPrefab);
		snapshot.SerializeVector(instance.m_vItemRotation);
		snapshot.SerializeInt(instance.m_iNominal);
		snapshot.SerializeInt(instance.m_iMinimum);
		snapshot.SerializeInt(instance.m_iLifetime);
		snapshot.SerializeInt(instance.m_iRestock);
		snapshot.SerializeInt(instance.m_iQuantityMinimum);
		snapshot.SerializeInt(instance.m_iQuantityMaximum);
		snapshot.SerializeInt(instance.m_ItemCategory);
		snapshot.SerializeInt(instance.m_ItemUsages);
		snapshot.SerializeInt(instance.m_ItemTiers);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet) 
	{
		snapshot.EncodeString(packet); // m_sName
		snapshot.EncodeString(packet); // m_sPrefab
		snapshot.EncodeVector(packet); // m_vItemRotation
		snapshot.EncodeInt(packet); // m_iNominal
		snapshot.EncodeInt(packet); // m_iMinimum
		snapshot.EncodeInt(packet); // m_iLifetime
		snapshot.EncodeInt(packet); // m_iRestock
		snapshot.EncodeInt(packet); // m_iQuantityMinimum
		snapshot.EncodeInt(packet); // m_iQuantityMaximum
		snapshot.EncodeInt(packet); // m_ItemCategory
		snapshot.EncodeInt(packet); // m_ItemUsages
		snapshot.EncodeInt(packet); // m_ItemTiers
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		snapshot.DecodeString(packet); // m_sName
		snapshot.DecodeString(packet); // m_sPrefab
		snapshot.DecodeVector(packet); // m_vItemRotation
		snapshot.DecodeInt(packet); // m_iNominal
		snapshot.DecodeInt(packet); // m_iMinimum
		snapshot.DecodeInt(packet); // m_iLifetime
		snapshot.DecodeInt(packet); // m_iRestock
		snapshot.DecodeInt(packet); // m_iQuantityMinimum
		snapshot.DecodeInt(packet); // m_iQuantityMaximum
		snapshot.DecodeInt(packet); // m_ItemCategory
		snapshot.DecodeInt(packet); // m_ItemUsages
		snapshot.DecodeInt(packet); // m_ItemTiers
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint) 
	{
		return lhs.CompareStringSnapshots(rhs) // m_sName
			&& lhs.CompareStringSnapshots(rhs) // m_sPrefab
			&& lhs.CompareSnapshots(rhs, 12) // m_vItemRotation
			&& lhs.CompareSnapshots(rhs, 4+4+4+4+4+4+4+4+4); // m_iNominal, m_iMinimum, m_iLifetime, m_iRestock, m_iQuantityMinimum, m_iQuantityMaximum, m_ItemCategory, m_ItemUsages, m_ItemTiers
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(CE_ItemData instance, SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		return snapshot.CompareString(instance.m_sName) 
			&& snapshot.CompareString(instance.m_sPrefab)
			&& snapshot.CompareVector(instance.m_vItemRotation) 
			&& snapshot.CompareInt(instance.m_iNominal) 
			&& snapshot.CompareInt(instance.m_iMinimum) 
			&& snapshot.CompareInt(instance.m_iLifetime) 
			&& snapshot.CompareInt(instance.m_iRestock)
			&& snapshot.CompareInt(instance.m_iQuantityMinimum)
			&& snapshot.CompareInt(instance.m_iQuantityMaximum)
			&& snapshot.CompareInt(instance.m_ItemCategory)
			&& snapshot.CompareInt(instance.m_ItemUsages)
			&& snapshot.CompareInt(instance.m_ItemTiers);
	}
	//################################################################################################
	*/
}

[BaseContainerProps(configRoot: true)]
class CE_ItemDataConfig
{
	[Attribute(UIWidgets.Object, desc: "Item Data")]
    ref array<ref CE_ItemData> m_ItemData;
	
	void ~CE_ItemDataConfig()
	{
	}
};







