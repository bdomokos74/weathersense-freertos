import os

Import("env")

env.BuildSources(
    os.path.join("$BUILD_DIR", "azure_ex", "azure-sdk-for-c", "sdk", "src", "azure", "core"),
    os.path.join("$PROJECT_DIR", "azure_ex", "azure-sdk-for-c", "sdk", "src", "azure", "core")
)

env.BuildSources(
    os.path.join("$BUILD_DIR", "azure_ex", "azure-sdk-for-c", "sdk", "src", "azure", "iot"),
    os.path.join("$PROJECT_DIR", "azure_ex", "azure-sdk-for-c", "sdk", "src", "azure", "iot")
)
