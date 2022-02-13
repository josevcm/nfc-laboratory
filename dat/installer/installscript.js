function Component()
{
}

Component.prototype.createOperations = function() {
    component.createOperations();
    component.addOperation("CreateShortcut", "@TargetDir@/nfc-lab.exe", "@StartMenuDir@/NFC-Laboratory.lnk", "workingDirectory=@TargetDir@");
    component.addOperation("CreateDesktopShortcut", "@TargetDir@/nfc-lab.exe", "@DesktopDir@/NFC-Laboratory.lnk", "workingDirectory=@TargetDir@");
}