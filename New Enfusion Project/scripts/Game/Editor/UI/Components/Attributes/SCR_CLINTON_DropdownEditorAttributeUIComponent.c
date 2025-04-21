/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes

Let's populate the menus with the factiosn

*/
class SCR_CLINTON_DropdownEditorAttributeUIComponent: SCR_DropdownEditorAttributeUIComponent
{	
	ref array<ref SCR_BaseEditorAttributeEntry> entries = {};
	ref array<ref string> entries_string = {};
	//============================ Init ============================\\
	/*!
	Initialize GUI from attribute.
	To be overridden by inherited classes.
	\param w Widget this component is attached to
	\param attribute Editor attribute this component represents
	*/
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		super.Init(w, attribute);
		//save_attribute = super.super.GetAttribute();
	}
	/*
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		//super.Init(w,attribute);
		
		array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
		
		FactionManager fm = GetGame().GetFactionManager();
		array<Faction> factions = {null};
		entries.Insert(new SCR_BaseEditorAttributeEntryText("Add to Each"));
		if (fm)
		{
			fm.GetFactionsList(factions);
		}
		if (factions)
		{
			foreach(Faction faction : factions)
			{
				entries.Insert(new SCR_BaseEditorAttributeEntryText(faction.GetFactionName()));
			}
		} else {
			Print("Can't find factions!", LogLevel.ERROR);
		}
		
		attribute.GetEntries(entries);	
	}
	*/
	//------------------------------------------------------------------------------------------------
	//If enabled UI is focused
	override bool OnFocus(Widget w, int x, int y)
	{
		// m_InputManager = GetGame().GetInputManager();
		//m_AttributeManager = new SCR_AttributesManagerEditorComponent;
		
		//return super.OnFocus(w,x,y);
		entries = new array<ref SCR_BaseEditorAttributeEntry>;
		entries_string.Clear();
		
		FactionManager fm = GetGame().GetFactionManager();
		array<Faction> factions = {null};
		entries.Insert(new SCR_BaseEditorAttributeEntryText("Add to Each"));
		if (fm)
		{
			fm.GetFactionsList(factions);
		}
		if (factions)
		{
			foreach(Faction faction : factions)  // TODO: avoid reseting everything on every OnFocus call
			{
				entries.Insert(new SCR_BaseEditorAttributeEntryText(faction.GetFactionName()));
				entries_string.Insert(faction.GetFactionName());
			}
		} else {
			Print("Can't find factions!", LogLevel.ERROR);
		}
		
		m_bIsFocused = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get entries to be shown in GUI lists.
	//! To be overridden by child classes.
	//! \outEntries Entries
	//! \return Number of entries
	int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		array<ref SCR_EditorAttributeFloatStringValueHolder> m_aValues = {};
		
		// Building all items in order to copy them to the Values variable.
		ref array<ref SCR_EditorAttributeFloatStringValueHolder> myValues = {};
		foreach (int index, string s : entries_string)
		{
			SCR_EditorAttributeFloatStringValueHolder myItem = BuildItem("",s,"",index);
			myValues.Insert(myItem);
		}
		
		m_aValues = myValues;
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
	
	// -----------------------------------------------------------------------------------------------------
	private SCR_EditorAttributeFloatStringValueHolder BuildItem(string icon, string name, string desc, float val)
	{
		SCR_EditorAttributeFloatStringValueHolder temp = new SCR_EditorAttributeFloatStringValueHolder;
		temp.SetIcon(icon);
		temp.SetName(name);
		temp.SetDescription(desc);
		temp.SetFloatValue(val);					
		return temp;
	}
};

	//------------------------------------------------------------------------------------------------
	//! Ripped from Discord
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SPK_MyUtilityEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	protected array<ref string> myStringArray;
	
	// -----------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		// Search your component in editableEntity.GetOwner()
		SPK_myMenuUI myComp = SPK_myMenuUI.Cast(editableEntity.GetOwner().FindComponent(SCR_CLINTON_DropdownEditorAttributeUIComponent));
		if (!myComp)
			return null;	
		
		// Get the info from your compoennt and save it to myStringArray to use it later in GetEntries() method.
		myStringArray = myComp.GetInfo();
		
		// Get current selected item in the list. (in some cases this may not have sense and you will set it to 0, but in this case the current item selected maters)
		int selectedItem = myComp.GetCurrentSelectedItem();
		
		return SCR_BaseEditorAttributeVar.CreateInt(selectedItem);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		// Search your component in editableEntity.GetOwner()
		SPK_myMenuUI myComp = SPK_myMenuUI.Cast(editableEntity.GetOwner().FindComponent(SCR_CLINTON_DropdownEditorAttributeUIComponent));
		if (!myComp)
			return;	
		
		// Set the new selection to the component
		myComp.SetCurrentSelectedItem(var.GetInt());
	}	
	
	// -----------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		m_aValues.Clear();
		
		// Building all items in order to copy them to the Values variable.
		ref array<ref SCR_EditorAttributeFloatStringValueHolder> myValues = {};
		foreach (int index, string s : myStringArray)
		{
			SCR_EditorAttributeFloatStringValueHolder myItem = BuildItem("",s,"",index);
			myValues.Insert(myItem);
		}
		
		m_aValues = myValues;
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}		
	
	// -----------------------------------------------------------------------------------------------------
	private SCR_EditorAttributeFloatStringValueHolder BuildItem(string icon, string name, string desc, float val)
	{
		SCR_EditorAttributeFloatStringValueHolder temp = new SCR_EditorAttributeFloatStringValueHolder;
		temp.SetIcon(icon);
		temp.SetName(name);
		temp.SetDescription(desc);
		temp.SetFloatValue(val);					
		return temp;
	}
}