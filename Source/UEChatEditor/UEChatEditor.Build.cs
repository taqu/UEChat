
namespace UnrealBuildTool.Rules
{
    public class UEChatEditor : ModuleRules
    {
        public UEChatEditor(ReadOnlyTargetRules Target) : base(Target)
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
                    "UnrealEd",
                    "SlateCore",
                    "Slate",
                    "EditorStyle",
                    "ToolMenus",
                    "InputCore",
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "UEChat",
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
