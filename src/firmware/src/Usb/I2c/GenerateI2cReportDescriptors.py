if __name__ == "__main__":
	print(
"""
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x00,                    // USAGE (Undefined)
	0x15, 0x00,                    // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              // LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    // REPORT_SIZE (8)
	0xa1, 0x01,                    // COLLECTION (Application)

	0x85, 0x01,                    //   REPORT_ID (1)
	0x95, 0x06,                    //   REPORT_COUNT (6)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x04,                    //   REPORT_ID (4)
	0x95, 0x0b,                    //   REPORT_COUNT (11)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x06,                    //   REPORT_ID (6)
	0x95, 0x1a,                    //   REPORT_COUNT (26)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x07,                    //   REPORT_ID (7)
	0x95, 0x1a,                    //   REPORT_COUNT (26)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x08,                    //   REPORT_ID (8)
	0x95, 0x0c,                    //   REPORT_COUNT (12)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x09,                    //   REPORT_ID (9)
	0x95, 0x0c,                    //   REPORT_COUNT (12)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x0a,                    //   REPORT_ID (10)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x0c,                    //   REPORT_ID (12)
	0x95, 0x06,                    //   REPORT_COUNT (6)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x10,                    //   REPORT_ID (16)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x11,                    //   REPORT_ID (17)
	0x95, 0x27,                    //   REPORT_COUNT (39)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x14,                    //   REPORT_ID (20)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x16,                    //   REPORT_ID (22)
	0x96, 0x04, 0x0c,              //   REPORT_COUNT (1036)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x18,                    //   REPORT_ID (24)
	0x95, 0x02,                    //   REPORT_COUNT (2)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x19,                    //   REPORT_ID (25)
	0x95, 0x27,                    //   REPORT_COUNT (39)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x1c,                    //   REPORT_ID (28)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)

	0x85, 0x1e,                    //   REPORT_ID (30)
	0x96, 0x04, 0x0c,              //   REPORT_COUNT (1036)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x82, 0x20, 0x01,              //   INPUT (Data,Ary,Abs,NPrf,Buf)
	0x09, 0x3a,                    //   USAGE (Counted Buffer)
	0x92, 0x20, 0x01,              //   OUTPUT (Data,Ary,Abs,NPrf,Buf)
""",
	end = '')

	for reportId in range(128, 256):
		reportCount = (reportId - 128) * 8 + 11
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
