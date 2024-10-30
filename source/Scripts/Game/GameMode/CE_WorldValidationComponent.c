[ComponentEditorProps(category: "CentralEconomy/Components", description: "")]
class CE_WorldValidationComponentClass: SCR_BaseGameModeComponentClass
{
};

class CE_WorldValidationComponent: SCR_BaseGameModeComponent
{
	protected bool m_Processed = false;
	
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		SetWorldProcessed(true);
	}
	
	static CE_WorldValidationComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return CE_WorldValidationComponent.Cast(GetGame().GetGameMode().FindComponent(CE_WorldValidationComponent));
		else
			return null;
	}
	
	bool GetWorldProcessed()
	{
		return m_Processed;
	}
	
	void SetWorldProcessed(bool meow)
	{
		m_Processed = meow;
	}
};