plugins {
    id("idea")
    id("org.jetbrains.intellij") version "1.10.1"
}

group = "com.huawei"
version = "1.1.6"
// TODO(vpukhov): adjust es2panda path
val migrator = "../../../../ecmascript/es2panda/migrator"

apply {
    from("${rootDir}/repos.gradle.kts")
}

dependencies {
    implementation("com.squareup.moshi:moshi:1.9.1")
}

// Configure Gradle IntelliJ Plugin
// Read more: https://plugins.jetbrains.com/docs/intellij/tools-gradle-intellij-plugin.html
intellij {
    version.set("2022.1.4")
    type.set("IU") // Target IDE Platform

    plugins.set(listOf("JavaScript"))
}

ant.importBuild("${migrator}/build.xml") {
    antTargetName ->  "ant-${antTargetName}"
}
ant.properties["basedir"] = migrator

task("buildLinter") {
    dependsOn("ant-clean", "ant-build")
    tasks.findByName("ant-build")?.mustRunAfter("ant-clean")
}

tasks {
    // Set the JVM compatibility versions
    withType<JavaCompile> {
        sourceCompatibility = "11"
        targetCompatibility = "11"
    }

    patchPluginXml {
        sinceBuild.set("212")
        untilBuild.set("231.*")
    }

    prepareSandbox {
        dependsOn("buildLinter")
        doLast {
            copy {
                val archive = fileTree("${migrator}/typescript/bundle").matching {
                    include("panda-ts-transpiler-*.tgz")
                }
                assert(archive.files.size == 1)

                from(tarTree(archive.files.iterator().next()))
                into(file("${buildDir}/idea-sandbox/plugins/${rootProject.name}/tool"))
            }
        }
    }
}
