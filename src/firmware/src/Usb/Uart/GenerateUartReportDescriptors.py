if __name__ == "__main__":
	print(
"""
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x00,                    // USAGE (Undefined)
	0x15, 0x00,                    // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              // LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    // REPORT_SIZE (8)
	0xa1, 0x01,                    // COLLECTION (Application)

	0x85, 0x40,                    //   REPORT_ID (64)
	0x95, 0x25,                    //   REPORT_COUNT (37)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)
""",
	end = '')

	for reportId in range(128, 256):
		reportCount = (reportId - 128) * 8 + 9
		print(
"""
	0x85, 0x{reportId:02x},                    //   REPORT_ID ({reportId})
	{reportCountItem}              //   REPORT_COUNT ({reportCount})
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)
""" \
		.format(
			reportId=reportId,
			reportCount=reportCount,
			reportCountItem=(
				"0x96, 0x{0:02x}, 0x{1:02x}," if reportCount > 0xff
				else "0x95, 0x{0:02x},      "
			).format(reportCount & 0xff, (reportCount & 0xff00) >> 8)
		),
	end = '')

	print(
"""
	0xc0                           // END_COLLECTION
"""		
	)
