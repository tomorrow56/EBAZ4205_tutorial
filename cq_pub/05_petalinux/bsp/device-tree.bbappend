FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " file://system-user.dtsi"

require ${@'device-tree-sdt.inc' if d.getVar('SYSTEM_DTFILE') != '' else ''}

python do_compile:prepend() {
    dtsi = d.expand("${TOPDIR}/../components/plnx_workspace/device-tree/device-tree/system-conf.dtsi")

    if os.path.exists(dtsi):
        bb.warn("Patching system-conf.dtsi: ps7_nand_0 -> nfc0")

        with open(dtsi, "r", encoding="utf-8") as f:
            text = f.read()

        new_text = text.replace("&ps7_nand_0 {", "&nfc0 {")

        if new_text != text:
            with open(dtsi, "w", encoding="utf-8") as f:
                f.write(new_text)
        else:
            bb.warn("ps7_nand_0 pattern was not found in system-conf.dtsi")
    else:
        bb.warn("system-conf.dtsi was not found: %s" % dtsi)
}
