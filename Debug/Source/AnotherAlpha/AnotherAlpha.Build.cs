using UnrealBuildTool;

public class AnotherAlpha : ModuleRules
{
    public AnotherAlpha(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "NetCore",
            "Networking"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "AnotherAlpha/Public"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}