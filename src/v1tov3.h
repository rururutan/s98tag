
static BYTE *v1tov3(BYTE *ps, DWORD slen, DWORD *pdlen)
{

	/* 150%(worst case) */
	DWORD dpos = 0x80;
	BYTE *pd;
	DWORD titlepos = GetDwordLE(ps + S98_OFS_OFSTITLE);
	size_t titlelen = titlepos?strlen((char*)(ps + titlepos)):0;

	pd = (BYTE *)malloc(dpos + 1024 + ((slen + slen + slen) >> 1) + titlelen + 1);
	if (!pd) return 0;

	// write header
	{
		memset(pd, 0, dpos);
		SetDwordBE(pd + S98_OFS_MAGIC, S98_MAGIC_V3);
		SetDwordLE(pd + S98_OFS_TIMER_INFO1, *(ps + S98_OFS_TIMER_INFO1));
		SetDwordLE(pd + S98_OFS_TIMER_INFO2, *(ps + S98_OFS_TIMER_INFO2));
		SetDwordLE(pd + S98_OFS_OFSDATA, 0x80);
		SetDwordLE(pd + S98_OFS_DEVICECOUNT, 1);
		SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x00, S98DEVICETYPE_OPNA);
		SetDwordLE(pd + S98_OFS_DEVICEINFO + 0x04, 7987200);
	}

	/* data copy */
	{
		DWORD psloop  = GetDwordLE(ps + S98_OFS_OFSLOOP);
		DWORD datapos = GetDwordLE(ps + S98_OFS_OFSDATA);
		DWORD spos    = datapos;

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
			memcpy(&pd[dpos], "machine=NEC PC-8801", strlen("machine=NEC PC-8801"));
		} else {
			memcpy(&pd[dpos], "machine=NEC PC-9801", strlen("machine=NEC PC-9801"));
		}
		dpos += (DWORD)strlen((char*)("machine=NEC PC-8801"));
		pd[dpos++] = 0x0a;
	}

	*pdlen = dpos;
	return pd;
}
