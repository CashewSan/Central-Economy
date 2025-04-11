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
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint) 
	{
		return lhs.CompareSnapshots(rhs, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(CE_ItemData prop, SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		return snapshot.Compare(prop.m_sName, 4) 
			&& snapshot.Compare(prop.m_sPrefab, 4)
			&& snapshot.Compare(prop.m_vItemRotation, 12) 
			&& snapshot.Compare(prop.m_sPrefab, 4) 
			&& snapshot.Compare(prop.m_sPrefab, 4) 
			&& snapshot.Compare(prop.m_sPrefab, 4) 
			&& snapshot.Compare(prop.m_sPrefab, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(CE_ItemData prop, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		//snapshot.SerializeBytes(prop.iVal, 4);
		//snapshot.SerializeBytes(prop.fVal, 4);
		//snapshot.SerializeBytes(prop.bVal, 1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, CE_ItemData prop) 
	{
		//snapshot.SerializeBytes(prop.iVal, 4);
		//snapshot.SerializeBytes(prop.fVal, 4);
		//snapshot.SerializeBytes(prop.bVal, 1);

		return true;
	}
	//################################################################################################
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







