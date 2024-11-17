function Component() {
}

Component.prototype.createOperations = function () {
    component.createOperations();
};

Component.prototype.installerFinished = function () {
    console.log("Installation finished!");
    QMessageBox.information("Installation", "The installation is complete!");
};