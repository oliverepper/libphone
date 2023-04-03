import org.gradle.internal.classpath.Instrumented

plugins {
    id("java")
    application
}

group = "de.oliver_epper.libphone"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
    flatDir {
        dirs("/home/andreasweber/Downloads/libphone/lib")
    }
}

dependencies {
    compileOnly(files( "libphone.so.0"))
    implementation("net.java.dev.jna:jna:5.13.0")
    testImplementation(platform("org.junit:junit-bom:5.9.1"))
    testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}

application {
    mainClass.set("de.oliver_epper.libphone.Main")
}

tasks.named<JavaExec>("run") {
    systemProperty("native-libs", "/<build output or download path>/lib/")
    standardInput = System.`in`
}