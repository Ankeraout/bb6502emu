g_tab = "    "
g_tabCount = 4
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
g_getTable = {
    "B": "l_lr35902->m_regB",
    "C": "l_lr35902->m_regC",
    "D": "l_lr35902->m_regD",
    "E": "l_lr35902->m_regE",
    "H": "l_lr35902->m_regH",
    "L": "l_lr35902->m_regL",
    "(HL)": "busRead(l_lr35902->m_bus, l_tmpAddress)",
    "A": "l_lr35902->m_regA"
}
g_operation = "l_lr35902->m_flagZ = ({:s} & (1 << {:d})) == 0;"

g_currentOpcode = 0x40

for l_bit in range(0, 8):
    for l_register in g_regTable:
        print(
            "{:s}case 0x{:02x}: // BIT {:d}, {:s}".format(
                g_tab * g_tabCount,
                g_currentOpcode,
                l_bit,
                l_register
            )
        )

        print(
            "{:s}l_tmpAddress = lr35902_getHL(l_lr35902);".format(
                g_tab * (g_tabCount + 1)
            )
        )

        print(
            "{:s}{:s}".format(
                g_tab * (g_tabCount + 1),
                g_operation.format(
                    g_getTable[l_register],
                    l_bit
                )
            )
        )
        print(
            "{:s}l_lr35902->m_flagN = false;".format(
                g_tab * (g_tabCount + 1)
            )
        )
        print(
            "{:s}l_lr35902->m_flagH = true;".format(
                g_tab * (g_tabCount + 1)
            )
        )

        print("{:s}break;".format(g_tab * (g_tabCount + 1)))

        print()

        g_currentOpcode += 1
