
static BYTE *v3tov1(BYTE *ps, DWORD slen, DWORD *pdlen)
{
	BYTE *pd = NULL;

	/* Check */
	{
		DWORD timerinfo2  = GetDwordLE(ps + S98_OFS_TIMER_INFO2);
		if (timerinfo2 != 0 && timerinfo2 != 1000) {
			// Can't convert
			return 0;
		}

		DWORD devcount = GetDwordLE(ps + S98_OFS_DEVICECOUNT);
		if (devcount != 0 && devcount != 1) {
			// Can't convert
			return 0;
		}

		if (devcount == 1) {
			DWORD devid = GetDwordLE(ps + S98_OFS_DEVICEINFO);
			if (devid != S98DEVICETYPE_OPNA) {
				return 0;
			}
		}
	}

	{
	/* 150%(worst case) */
	DWORD dpos = 0x80;
	DWORD titlepos = GetDwordLE(ps + S98_OFS_OFSTITLE);
	size_t titlelen = titlepos?strlen((char*)(ps + titlepos)):0;

	pd = (BYTE *)malloc(dpos + 1024 + ((slen + slen + slen) >> 1) + titlelen + 1);
	if (!pd) return 0;

	/* write header */
	{
		memset(pd, 0, dpos);
		SetDwordBE(pd + S98_OFS_MAGIC, S98_MAGIC_V1);
		SetDwordLE(pd + S98_OFS_TIMER_INFO1, *(ps + S98_OFS_TIMER_INFO1));
		SetDwordLE(pd + S98_OFS_OFSDATA, 0x80);
	}

	/* Data copy */
	{
		DWORD psloop  = GetDwordLE(ps + S98_OFS_OFSLOOP);
		DWORD datapos = GetDwordLE(ps + S98_OFS_OFSDATA);
		DWORD tagpos  = GetDwordLE(ps + S98_OFS_OFSTITLE);
		DWORD spos    = datapos;
		if (spos < tagpos)
			slen = tagpos;

		BYTE dataend = 0;
		while (spos < slen && dataend == 0) {
			if (psloop == spos) {
				if (psloop) SetDwordLE(pd + S98_OFS_OFSLOOP, 0x80 + (spos - datapos));
			}

			switch (ps[spos]) {
			  case 0:
			  case 1:
				spos += 3;
				break;
			  case 0xff:
				spos += 1;
				break;
			  case 0xfe:
				spos += 1;
				while (ps[spos] & 0x80) {
					spos += 1;
				}
				spos +=1;
				break;
			  case 0xfd:
				spos += 1;
				dataend = 1;
				break;
			}
		}
		memcpy(&pd[0x80], ps + datapos, spos - datapos);
		dpos += spos - datapos;

		// 終端が終端コマンドで無い場合は終端コマンドを追加する
		if (pd[dpos-1] != 0xfd) {
			pd[dpos++] = 0xfd;
		}
	}

#if 0
	/* write tag */
	{
		if (titlepos) {
			SetDwordLE(pd + S98_OFS_OFSTITLE, dpos);
			memcpy(&pd[dpos], "[S98]title=", strlen("[S98]title="));
			dpos += (DWORD)strlen("[S98]title=");
			memcpy(&pd[dpos], &ps[titlepos], titlelen);
			dpos += (DWORD)titlelen;
			pd[dpos++] = 0x0a;
		}
		if (GetDwordLE(ps + S98_OFS_TIMER_INFO1) == 0) {
			memcpy(&pd[dpos], "system=NEC PC-8801", strlen("machine=NEC PC-8801"));
		} else {
			memcpy(&pd[dpos], "system=NEC PC-9801", strlen("machine=NEC PC-9801"));
		}
		dpos += (DWORD)strlen((char*)("machine=NEC PC-8801"));
		pd[dpos++] = 0x0a;
	}
#endif
	*pdlen = dpos;
	}

	return pd;
}
