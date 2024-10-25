
namespace UnrealBuildTool.Rules
{
    public class UEChat : ModuleRules
    {
        public UEChat(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.NoSharedPCHs;

            PublicIncludePaths.AddRange(
                new string[] {
                }
                );

            PrivateIncludePaths.AddRange(
                new string[] {
                }
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Settings",
                    "HTTP",
                    "Json",
                    "JsonUtilities",
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                }
                );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                }
                );
        }
    }
}
