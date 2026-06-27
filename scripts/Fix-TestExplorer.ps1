$targetDir = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\Extensions\TestPlatform\Extensions"

# EBF assemblies
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\EBF\Microsoft.VisualStudio.Workspace.ExternalBuildFramework.Impl.dll" $targetDir -Force
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\EBF\Microsoft.VisualStudio.Workspace.ExternalBuildFramework.TestContracts.dll" $targetDir -Force

# Workspace assemblies
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\OpenFolder\Microsoft.VisualStudio.Workspace.dll" $targetDir -Force
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\OpenFolder\Microsoft.VisualStudio.Workspace.Extensions.dll" $targetDir -Force

# VC assemblies
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VC.Interfaces.dll" $targetDir -Force
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\PrivateAssemblies\Microsoft.VisualStudio.VisualC.Utilities.dll" $targetDir -Force
Copy-Item "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\PrivateAssemblies\Microsoft.VisualStudio.CppSvc.Internal.dll" $targetDir -Force

