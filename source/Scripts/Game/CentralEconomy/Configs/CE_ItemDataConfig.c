[BaseContainerProps(), BaseContainerCustomTitleField("m_sName")]
class CE_ItemData
{
	[Attribute("", UIWidgets.Auto, desc: "Name of Item **MUST BE UNIQUE**")]
	string m_sName;
	
    [Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Prefab to be spawned", "et")]
    ResourceName m_sPrefab;
	
	[Attribute("0 0 0", UIWidgets.EditBox, desc: "Euler angles of the item upon spawning (E.G. Weapons are typically x:0 y:0 z:90)")]
	vector m_vItemRotation;
	
	[Attribute("2", UIWidgets.EditBox, params: "1 10000", desc: "Ideal amount of spawns of this item around the map")]
	int m_iNominal;
	
	[Attribute("1", UIWidgets.EditBox, params: "1 10000", desc: "Minimum amount of spawns of this item around the map **SHOULD BE THE SAME OR LESS THAN THE NOMINAL VALUE**")]
	int m_iMinimum;
	
	[Attribute("14400", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) item can stay on at a spawner before despawning")]
	int m_iLifetime;
	
	[Attribute("3600", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) for item single instance to be added back to item pool after removal from spawner")]
	int m_iRestock;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Maximum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values)")]
	int m_iQuantityMaximum;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Minimum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values) **SHOULD BE THE SAME OR LESS THAN THE QUANTITY MAXIMUM VALUE**")]
	int m_iQuantityMinimum;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which category does this item fall under?", enums: ParamEnumArray.FromEnum(CE_ELootCategory))]
	CE_ELootCategory m_ItemCategory;
	
	[Attribute("", UIWidgets.Flags, desc: "Which spawn location(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootUsage))]
	CE_ELootUsage m_ItemUsages;
	
	[Attribute("", UIWidgets.Flags, desc: "Which tier(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootTier))]
	CE_ELootTier m_ItemTiers;
	
	static const int DATA_SIZE_EXCLUDE_STRINGS = 48;
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_sName);
		writer.WriteResourceName(m_sPrefab);
		writer.Write(m_vItemRotation, 96);
		writer.Write(m_iNominal, 32);
		writer.Write(m_iMinimum, 32);
		writer.Write(m_iLifetime, 32);
		writer.Write(m_iRestock, 32);
		writer.Write(m_iQuantityMaximum, 32);
		writer.Write(m_iQuantityMinimum, 32);
		writer.Write(m_ItemCategory, 32);
		writer.Write(m_ItemUsages, 32);
		writer.Write(m_ItemTiers, 32);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadString(m_sName);
		reader.ReadResourceName(m_sPrefab);
		reader.Read(m_vItemRotation, 96);
		reader.Read(m_iNominal, 32);
		reader.Read(m_iMinimum, 32);
		reader.Read(m_iLifetime, 32);
		reader.Read(m_iRestock, 32);
		reader.Read(m_iQuantityMaximum, 32);
		reader.Read(m_iQuantityMinimum, 32);
		reader.Read(m_ItemCategory, 32);
		reader.Read(m_ItemUsages, 32);
		reader.Read(m_ItemTiers, 32);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(CE_ItemData prop, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeString(prop.m_sName);
		snapshot.SerializeString(prop.m_sPrefab);
		snapshot.SerializeBytes(prop.m_vItemRotation, 12);
		snapshot.SerializeBytes(prop.m_iNominal, 4);
		snapshot.SerializeBytes(prop.m_iMinimum, 4);
		snapshot.SerializeBytes(prop.m_iLifetime, 4);
		snapshot.SerializeBytes(prop.m_iRestock, 4);
		snapshot.SerializeBytes(prop.m_iQuantityMaximum, 4);
		snapshot.SerializeBytes(prop.m_iQuantityMinimum, 4);
		snapshot.SerializeBytes(prop.m_ItemCategory, 4);
		snapshot.SerializeBytes(prop.m_ItemUsages, 4);
		snapshot.SerializeBytes(prop.m_ItemTiers, 4);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CE_ItemData prop)
	{
		if (!prop)
			return false;
		
		snapshot.SerializeString(prop.m_sName);
		snapshot.SerializeString(prop.m_sPrefab);
		snapshot.SerializeBytes(prop.m_vItemRotation, 12);
		snapshot.SerializeBytes(prop.m_iNominal, 4);
		snapshot.SerializeBytes(prop.m_iMinimum, 4);
		snapshot.SerializeBytes(prop.m_iLifetime, 4);
		snapshot.SerializeBytes(prop.m_iRestock, 4);
		snapshot.SerializeBytes(prop.m_iQuantityMaximum, 4);
		snapshot.SerializeBytes(prop.m_iQuantityMinimum, 4);
		snapshot.SerializeBytes(prop.m_ItemCategory, 4);
		snapshot.SerializeBytes(prop.m_ItemUsages, 4);
		snapshot.SerializeBytes(prop.m_ItemTiers, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
		snapshot.Serialize(packet, CE_ItemData.DATA_SIZE_EXCLUDE_STRINGS);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		snapshot.Serialize(packet, CE_ItemData.DATA_SIZE_EXCLUDE_STRINGS);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareSnapshots(rhs, CE_ItemData.DATA_SIZE_EXCLUDE_STRINGS);
	}

	//------------------------------------------------------------------------------------------------
	//! Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(CE_ItemData prop, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		if (!prop)
			return false;
		
		return snapshot.CompareString(prop.m_sName)
			&& snapshot.CompareString(prop.m_sPrefab)
			&& snapshot.Compare(prop.m_vItemRotation, 12)
			&& snapshot.Compare(prop.m_iNominal, 4)
			&& snapshot.Compare(prop.m_iMinimum, 4)
			&& snapshot.Compare(prop.m_iLifetime, 4)
			&& snapshot.Compare(prop.m_iRestock, 4)
			&& snapshot.Compare(prop.m_iQuantityMaximum, 4)
			&& snapshot.Compare(prop.m_iQuantityMinimum, 4)
			&& snapshot.Compare(prop.m_ItemCategory, 4)
			&& snapshot.Compare(prop.m_ItemUsages, 4)
			&& snapshot.Compare(prop.m_ItemTiers, 4);
	}
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







