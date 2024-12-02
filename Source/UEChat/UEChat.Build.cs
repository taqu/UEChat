using System.IO;

namespace UnrealBuildTool.Rules
{
    public class UEChat : ModuleRules
    {
        private string LlamaCppRootDirectory { get { return Path.Combine(PluginDirectory, "llama.cpp"); } }

        protected string LlamaCppIncludeDirectory
        {
            get
            {
                return Path.Combine(LlamaCppRootDirectory, "include");
            }
        }

        protected string LlamaCppLibraryDirectory
        {
            get
            {
                return Path.Combine(LlamaCppRootDirectory, "lib");
            }
        }

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
            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicSystemIncludePaths.Add(LlamaCppIncludeDirectory);
                string LibDirectory = LlamaCppLibraryDirectory;
                switch (Target.Configuration)
                {
                    case UnrealTargetConfiguration.Debug:
                    case UnrealTargetConfiguration.DebugGame:
                        PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "ggmld.lib"));
                        PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "llamad.lib"));
                        break;
                    case UnrealTargetConfiguration.Development:
                    case UnrealTargetConfiguration.Test:
                        PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "ggml.lib"));
                        PublicAdditionalLibraries.Add(Path.Combine(LibDirectory, "llama.lib"));
                        break;
                }
            }
        }
    }
}
