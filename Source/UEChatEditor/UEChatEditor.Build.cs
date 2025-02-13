
namespace UnrealBuildTool.Rules
{
    public class UEChatEditor : ModuleRules
    {
        public UEChatEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.NoSharedPCHs;
            PrivatePCHHeaderFile = "Private/UEChatEditorPrivatePCH.h";
            CppStandard = CppStandardVersion.Cpp20;

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
