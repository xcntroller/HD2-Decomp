// Model the verified x86 stack-probe helper in the base-game research project.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;

public class ConfigureStackProbe extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())) {
            throw new IllegalArgumentException("This script is specific to the unpacked base client");
        }
        byte[] expected = {
            0x51, 0x3d, 0x00, 0x10, 0x00, 0x00, (byte) 0x8d, 0x4c, 0x24, 0x08
        };
        for (int i = 0; i < expected.length; i++) {
            if (getByte(toAddr(0x7e5f60L + i)) != expected[i]) {
                throw new IllegalStateException("Stack-probe helper bytes do not match");
            }
        }
        Function probe = getFunctionAt(toAddr(0x7e5f60L));
        if (probe == null) {
            throw new IllegalStateException("Missing stack-probe function");
        }
        println("Previous stack-probe call fixup: " + probe.getCallFixup());
        probe.setCallFixup("alloca_probe");
        println("Applied built-in alloca_probe model at " + probe.getEntryPoint());
    }
}
