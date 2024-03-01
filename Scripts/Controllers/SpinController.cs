using UnityEngine;
using System.Collections;
using dcmns;

//Attach this to a 3d model game object
public class SpinController : MonoBehaviour
{
	Vector3 axis;
	public float Speed = 50.0f;
	private bool spinEnabled = true;
	private bool resetRotation = false;
	private bool serverRunning = false;
	DebugConnectionManager dcm = null;
	string spinLabel;
	string serverLabel;
	Quaternion initialLocalRotation;

	void Awake()
	{
		initialLocalRotation = transform.localRotation;
    	}

	void Start()
	{
		dcm = DebugConnectionManager.GetInstance();
		dcm.AddRule("spin", ControlSpin);
		dcm.AddRule("speed", ControlSpin);
		dcm.AddRule("axis", SetAxis);
		dcm.SetLogger(LogMessage);
		spinLabel = "reset spin";
		serverLabel = "Start";
		axis = Vector3.up;
		spinEnabled = true;
		serverRunning = false;
	}

	// Update is called once per frame
	void Update()
	{
		if (resetRotation)
		{ 
			transform.localRotation = initialLocalRotation;
			resetRotation = false;
		}
		
		if (spinEnabled)
		{
			float anglularDispl = Speed * Time.deltaTime;
			transform.Rotate(axis, anglularDispl);
		}
	}

	public int SetAxis(string[] commandAndArgs)
	{
		float x = 0, y = 1, z = 0;
		if (commandAndArgs.Length == 4)
		{
			try
			{
				x = float.Parse(commandAndArgs[1], System.Globalization.CultureInfo.InvariantCulture.NumberFormat);
				y = float.Parse(commandAndArgs[2], System.Globalization.CultureInfo.InvariantCulture.NumberFormat);
				z = float.Parse(commandAndArgs[3], System.Globalization.CultureInfo.InvariantCulture.NumberFormat);
			}
			catch (System.Exception ex)
			{
				Debug.Log(ex.ToString());
				return 1;
			}
		}
		else
		{
			return 1;
		}
		if(x==0 && y==0 && z == 0)
		{
			resetRotation = true;
			axis = new Vector3(0, 1, 0);
		}
		else 
		{
			axis = new Vector3(x, y, z);
		}
		return 0;
	}

	public int ControlSpin(string[] commandAndArgs)
	{
		if (commandAndArgs[0].Equals("spin") && commandAndArgs[1].Equals("false"))
		{
			spinEnabled = false;
			return 0;
		}
		else if (commandAndArgs[0].Equals("spin") && commandAndArgs[1].Equals("true"))
		{
			spinEnabled = true;
			return 0;
		}
		else if (commandAndArgs[0].Equals("speed"))
		{
			float newspeed=Speed;
			try
			{
				newspeed = float.Parse(commandAndArgs[1], System.Globalization.CultureInfo.InvariantCulture.NumberFormat);
			}
			catch (System.Exception e)
			{
				Debug.Log(e.ToString());
				Debug.Log("@controlSpin(..): cannot parse number string");
				return 1;
			}
			Speed = newspeed;
			return 0;
		}
		return 1;
	}
	
	void OnGUI()
	{
		if (GUI.Button(new Rect(100, 100, 100, 75), spinLabel))
		{
			resetRotation = true;
			axis = new Vector3(0, 1, 0);
			Speed = 50.0f;
		}
		if (GUI.Button(new Rect(100, 100 + 80, 100, 75), serverLabel))
		{
			if (serverRunning)
			{
				dcm.End();
				serverLabel = "Start";
				serverRunning = false;
			}
			else
			{
				dcm.Begin();
				serverLabel = "Stop";
				serverRunning = true;
			}
		}
	}

	public void LogMessage(string message, LogLevel level)
	{
		if (level == LogLevel.ERROR)
		{
			Debug.LogError(message);
		}
		else
		{
			Debug.Log(message);
		}
	}

	void OnApplicationQuit()
	{
		dcm.End();
	}
}
