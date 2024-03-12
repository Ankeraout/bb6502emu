g_tab = "    "
g_tabCount = 4
g_regTable = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
g_operationTable = ["RES", "SET"]
g_codeTable = {
    "RES": "{:s} & ~(1 << {:d})",
    "SET": "{:s} | (1 << {:d})"
}
g_setTable = {
    "B": "l_lr35902->m_regB = {:s};",
    "C": "l_lr35902->m_regC = {:s};",
    "D": "l_lr35902->m_regD = {:s};",
    "E": "l_lr35902->m_regE = {:s};",
    "H": "l_lr35902->m_regH = {:s};",
    "L": "l_lr35902->m_regL = {:s};",
    "(HL)": "busWrite(l_lr35902->m_bus, l_tmpAddress, {:s});",
    "A": "l_lr35902->m_regA = {:s};"
}
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

g_currentOpcode = 0x80

for l_operation in g_operationTable:
    for l_bit in range(0, 8):
        for l_register in g_regTable:
            print(
                "{:s}case 0x{:02x}: // {:s} {:d}, {:s}".format(
                    g_tab * g_tabCount,
                    g_currentOpcode,
                    l_operation,
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
                    g_setTable[l_register].format(
                        g_codeTable[l_operation].format(
                            g_getTable[l_register],
                            l_bit
                        )
                    )
                )
            )

            print("{:s}break;".format(g_tab * (g_tabCount + 1)))

            print()

            g_currentOpcode += 1