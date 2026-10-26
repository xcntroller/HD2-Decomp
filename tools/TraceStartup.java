// Decode the startup prefix independently of saved instruction boundaries.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.app.util.PseudoDisassembler;
import ghidra.app.util.PseudoInstruction;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

public class TraceStartup extends GhidraScript {
    private String inlineString(Address start, long length) throws Exception {
        if (length < 4 || length > 128) {
            return null;
        }
        StringBuilder text = new StringBuilder();
        for (int i = 0; i < length; i++) {
            int value = currentProgram.getMemory().getByte(start.add(i)) & 255;
            if (i == length - 1) {
                return value == 0 ? text.toString() : null;
            }
            if (value < 32 || value > 126) {
                return null;
            }
            text.append((char) value);
        }
        return null;
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 1) {
            throw new IllegalArgumentException("Supply a new output filename");
        }
        try (PrintWriter out = new PrintWriter(Files.newBufferedWriter(
                Path.of(args[0]), StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW))) {
            PseudoDisassembler decoder = new PseudoDisassembler(currentProgram);
            Address cursor = toAddr(0x996a05L);
            out.println("Read-only decoding; inline-string calls followed at their targets.");
            out.println("Ordinary calls are not followed; this is not runtime execution.");
            int strings = 0;
            for (int i = 0; i < 180 && !monitor.isCancelled(); i++) {
                PseudoInstruction instruction = decoder.disassemble(cursor);
                if (instruction == null) {
                    throw new IllegalStateException("Cannot decode " + cursor);
                }
                out.println(cursor + " " + instruction);
                Address next = cursor.add(instruction.getLength());
                if (instruction.getFlowType().isCall() &&
                        (currentProgram.getMemory().getByte(cursor) & 255) == 0xe8) {
                    Address target = instruction.getAddress(0);
                    String text = target == null ? null : inlineString(next, target.subtract(next));
                    if (text == null) {
                        out.println("STOP: ordinary direct call requires separate control-flow analysis.");
                        break;
                    }
                    out.println("  inline argument string at " + next + ": " + text);
                    strings++;
                    cursor = target;
                }
                else {
                    if (instruction.getFlowType().isJump() ||
                            instruction.getFlowType().isTerminal()) {
                        out.println("STOP: branch/terminal requires separate control-flow analysis.");
                        break;
                    }
                    cursor = next;
                }
            }
            out.println("Recognized inline strings: " + strings);
            DecompInterface decompiler = new DecompInterface();
            try {
                if (!decompiler.openProgram(currentProgram)) {
                    throw new IllegalStateException(decompiler.getLastMessage());
                }
                FunctionIterator functions = currentProgram.getFunctionManager().getFunctions(true);
                int tested = 0;
                while (functions.hasNext() && tested < 3 && !monitor.isCancelled()) {
                    Function function = functions.next();
                    long entry = function.getEntryPoint().getOffset();
                    long size = function.getBody().getNumAddresses();
                    if (entry < 0x401000L || entry >= 0x80f000L || size < 48 || size > 256) {
                        continue;
                    }
                    DecompileResults result = decompiler.decompileFunction(function, 20, monitor);
                    out.println("Main .text sample " + function.getEntryPoint() +
                        " size=" + size + " decompiled=" + result.decompileCompleted() +
                        " message=" + result.getErrorMessage());
                    tested++;
                }
                if (tested == 0) {
                    throw new IllegalStateException("No main .text function samples found");
                }
            }
            finally {
                decompiler.dispose();
            }
            if (out.checkError()) {
                throw new IllegalStateException("Failed writing startup trace");
            }
        }
        println("Wrote independent startup trace to " + args[0]);
    }
}
