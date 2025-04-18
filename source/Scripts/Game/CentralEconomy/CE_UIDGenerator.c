class CE_HexHelper
{
	//------------------------------------------------------------------------------------------------
	//! Convert integer to hexadeciamal string
	//! \param value Input integer
	//! \param upperCase Decide if output should be upper or lower case
	//! \param fixedLength Add leading zeros for a minimum length output
	//! \return result hexadecimal string
	static string Convert(int value, bool upperCase = false, int fixedLength = -1)
	{
		array<string> resultChars = {"0", "0", "0", "0", "0", "0", "0", "0"};

		int asciiOffset = 87;
		if (upperCase) asciiOffset = 55;

		int padUntil = 7;
		if (fixedLength != -1) padUntil = 8 - Math.Min(fixedLength, 8);

		int resultIdx = 7;

		while (value)
		{
			int remainder = value % 16;

			if (remainder < 10)
			{
				resultChars.Set(resultIdx--, remainder.ToString());
			}
			else
			{
				resultChars.Set(resultIdx--, (remainder + asciiOffset).AsciiToString());
			}

			value /= 16;
		}

		string result;
		bool nonZero;

		foreach (int nChar, string char : resultChars)
		{
			if (char == "0" && nChar < padUntil && !nonZero) continue;
			nonZero = true;
			result += char;
		}

		return result;
	}
};

class CE_UIDGenerator
{
	protected static int s_iSequence;
	protected static ref RandomGenerator s_pRandom;

	//------------------------------------------------------------------------------------------------
	static string Generate()
	{
		if (!s_pRandom)
		{
			s_pRandom = new RandomGenerator();
			s_pRandom.SetSeed(System.GetUnixTime());
		}

		//No need to look at past generated ids in db for conflict, because they have older timestamps
		string timeHex = CE_HexHelper.Convert(System.GetUnixTime(), false, 8);

		//Sequence returns to 0 on overflow
		string seqHex = CE_HexHelper.Convert(s_iSequence++, false, 8);
		if (s_iSequence == int.MAX)
			s_iSequence = 0;

		//Random component to ensure that even if all sequentials are generated in the same second it stays unique
		string randHex = CE_HexHelper.Convert(s_pRandom.RandFloatXY(0, int.MAX), false, 8);
		string randHex2 = CE_HexHelper.Convert(s_pRandom.RandFloatXY(0, int.MAX), false, 8);

		// TTTT TTTT-SEQ1-SEQ1-RND1-RND1 RND2 RND2
		return string.Format("%1-%2-%3-%4-%5%6",
			timeHex,
			seqHex.Substring(0, 4),
			seqHex.Substring(4, 4),
			randHex.Substring(0, 4),
			randHex.Substring(4, 4),
			randHex2);
	}

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_iSequence = 0;
		s_pRandom = null;
	}
};

// Portions pulled and modified from https://github.com/Arkensor/EnfusionDatabaseFramework/blob/armareforger/src/Scripts/Game/EDF_DbEntityIdGenerator.c