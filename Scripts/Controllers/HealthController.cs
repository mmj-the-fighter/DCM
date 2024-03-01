using UnityEngine;
using System;
using dcmns;

//Attach this to an empty gameobject
public class HealthController : MonoBehaviour
{
	DebugConnectionManager dcm = null;
	int health;
	string healthstr;
	GUIStyle guiStyle = new GUIStyle();
  
	void Start()
	{
		dcm = DebugConnectionManager.GetInstance();
		dcm.AddRule("health", UpdateHealth);
		health = 100;
		GenerateHealthStr();
		guiStyle.fontSize = 40;
		guiStyle.normal.textColor = Color.white;
	}
	void GenerateHealthStr()
	{
		healthstr = "health " + health.ToString();
	}
	
	public int UpdateHealth(string[] commandsAndArgs)
	{
		int dh;
		try 
		{
			dh = Int32.Parse(commandsAndArgs[1]);
		}
		catch(Exception e)
		{
			Debug.Log(e.ToString());
			Debug.Log("@updateHealth(..): cannot parse number string");
			return 1;
		}
		health += dh;
		GenerateHealthStr();
		return 0;
	}

	void OnGUI()
	{
		GUI.Label(new Rect(100 + 75 * 4, 100, 100, 75), healthstr, guiStyle);
	}
}
