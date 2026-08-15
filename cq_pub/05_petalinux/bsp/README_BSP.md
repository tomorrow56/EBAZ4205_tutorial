# EBAZ4205のPetaLinuxプロジェクトからBSPを生成する

## Device Treeの`system-conf.dtsi`の`&ps7_nand_0 {`を`&nfc0 {`へ自動置換

```bash
cd ~/petalinux2024.2/ebaz4205_linux
vi project-spec/meta-user/recipes-bsp/device-tree/device-tree.bbappend
```

`device-tree.bbappend`を以下のように修正する

```
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
```

## `petalinux-package`のバグ対応

参考:

https://adaptivesupport.amd.com/s/question/0D54U00008X0fsSSAR/petalinux-20241-petalinuxpackage-fails-with-error-unable-to-create-directory-at-build?language=en_US

https://github.com/Xilinx/PetaLinux/pull/4/changes/23e00d30c4d0dcf761d05422393717308768ac34#diff-d6c7ab5441539ae8e5d39241600968f727ca32c74641dedd3147ccb506bdc649L123-R133

`scripts/petalinux-package`に以下を追記

```
 proot = plnx_utils.exit_not_plnx_project(args.project)
        else:
            proot = plnx_utils.exit_not_plnx_project(proot='')
+    else:
+        # Exit if PROOT is not PetaLinux project
+        if args.project:
+            proot = plnx_utils.exit_not_plnx_project(args.project[0])
+        else:
+            proot = plnx_utils.exit_not_plnx_project(proot='')

    args.builddir = plnx_vars.BuildDir.format(proot)
    plnx_utils.CreateDir(args.builddir)
```


## petalinux-package bsp コマンド

```
petalinux-package bsp -p <PATH-TO-PROJECT> -o <PATH-TO-BSP>
```

