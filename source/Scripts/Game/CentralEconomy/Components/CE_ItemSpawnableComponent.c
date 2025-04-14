[ComponentEditorProps(category: "CentralEconomy/Components", description: "Component to added to spawnable items (mostly for tracking UID)")]
class CE_ItemSpawnableComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawnableComponent : ScriptComponent
{
	protected string m_iItemUID;
	
	//------------------------------------------------------------------------------------------------
	//! Gets the usage of the spawning component
	string GetItemUID()
	{
		return m_iItemUID;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Sets the usage of the spawning component
	void SetItemUID(string uid)
	{
		m_iItemUID = uid;
	}
}