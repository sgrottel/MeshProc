using System.Text.Json.Serialization;

namespace PlySelectorReceiver
{

	/*
	Examples:

	{
		"normal": [
		0.0,
		0.0,
		1.0
		],
		"point": [
		0.0,
		0.0,
		0.0
		],
		"selection": "plane"
	}

	{
	  "end": [
		-21.669384002685547,
		68.01781463623047,
		-80.63137817382813
	  ],
	  "hit1": [
		-15.074535369873047,
		47.31730651855469,
		78.6258544921875
	  ],
	  "hit2": [
		-20.760862350463867,
		65.16606140136719,
		-58.69172668457031
	  ],
	  "selection": "ray",
	  "start": [
		-14.991487503051758,
		47.0566291809082,
		80.63134765625
	  ]
	}

	*/

	public class SelectionData
	{
		[JsonPropertyName("selection")]
		public string? Selection { get; set; }

		[JsonPropertyName("start")]
		public float[]? Start { get; set; }

		[JsonPropertyName("end")]
		public float[]? End { get; set; }

		[JsonPropertyName("hit1")]
		public float[]? Hit1 { get; set; }

		[JsonPropertyName("hit2")]
		public float[]? Hit2 { get; set; }

		[JsonPropertyName("normal")]
		public float[]? Normal { get; set; }

		[JsonPropertyName("point")]
		public float[]? Point { get; set; }

		public bool IsValid()
		{
			if (Selection == "plane")
			{
				if (Normal == null) { return false; }
				if (Normal.Length != 3) { return false; }
				if (Point == null) { return false; }
				if (Point.Length != 3) { return false; }
			}
			else if (Selection == "ray")
			{
				if (Start == null) { return false; }
				if (Start.Length != 3) { return false; }
				if (End == null) { return false; }
				if (End.Length != 3) { return false; }
				if (Hit1 == null) { return false; }
				if (Hit1.Length != 3) { return false; }
				if (Hit2 == null) { return false; }
				if (Hit2.Length != 3) { return false; }
			}
			else
			{
				return false;
			}
			return true;
		}

		public override string ToString()
		{
			if (Selection == "plane")
			{
				return $"Plane Selection ({Point?[0]}, {Point?[1]}, {Point?[2]}) × ({Normal?[0]}, {Normal?[1]}, {Normal?[2]})";
			}
			else if (Selection == "ray")
			{
				return $"Ray Selection ({Start?[0]}, {Start?[1]}, {Start?[2]}) - ({End?[0]}, {End?[1]}, {End?[2]})";
			}
			else
			{
				return "Invalid Selection";
			}
		}
	}
}
