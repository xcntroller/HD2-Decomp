// Inspect saved startup analysis without changing the program.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Bookmark;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.mem.MemoryBlock;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Iterator;

public class InspectStartup extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1 || args.length > 2) {
            throw new IllegalArgumentException("Supply a new output filename and optional address");
        }
        try (PrintWriter out = new PrintWriter(Files.newBufferedWriter(
                Path.of(args[0]), StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW))) {
            out.println("Program: " + currentProgram.getName());
            out.println("Executable: " + currentProgram.getExecutablePath());
            out.println("SHA256: " + currentProgram.getExecutableSHA256());
            out.println("Language: " + currentProgram.getLanguageID());
            out.println("Functions: " + currentProgram.getFunctionManager().getFunctionCount());
            for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
                out.println("Block: " + block.getName() + " " + block.getStart() +
                    "-" + block.getEnd() + " execute=" + block.isExecute());
            }
            Address entry = args.length == 2 ? toAddr(args[1]) : toAddr(0x996a05L);
            MemoryBlock entryBlock = currentProgram.getMemory().getBlock(entry);
            if (entryBlock == null) {
                throw new IllegalArgumentException("Address is outside program memory: " + entry);
            }
            Function function = getFunctionContaining(entry);
            out.println("Entry function: " + (function == null ? "none" :
                function.getName() + " at " + function.getEntryPoint() +
                " body bytes=" + function.getBody().getNumAddresses()));
            out.println("\nSaved listing at entry:");
            Address cursor = entry;
            for (int i = 0; i < 110 && !monitor.isCancelled(); i++) {
                CodeUnit unit = currentProgram.getListing().getCodeUnitAt(cursor);
                if (unit == null) {
                    out.printf("%s undefined byte %02x%n", cursor,
                        currentProgram.getMemory().getByte(cursor) & 255);
                    cursor = cursor.add(1);
                }
                else {
                    out.println(cursor + " " + unit);
                    if (unit instanceof Instruction instruction) {
                        out.println("  flow=" + instruction.getFlowType() +
                            " fallthrough=" + instruction.getFallThrough());
                    }
                    cursor = unit.getMaxAddress().add(1);
                }
            }
            out.println("\nError bookmarks in startup section:");
            Iterator<Bookmark> bookmarks = currentProgram.getBookmarkManager().getBookmarksIterator();
            int errors = 0;
            while (bookmarks.hasNext() && errors < 20) {
                Bookmark bookmark = bookmarks.next();
                if ("Error".equals(bookmark.getTypeString()) &&
                        entryBlock.contains(bookmark.getAddress())) {
                    out.println(bookmark.getAddress() + " " + bookmark.getCategory() +
                        ": " + bookmark.getComment());
                    errors++;
                }
            }
            if (function != null) {
                DecompInterface decompiler = new DecompInterface();
                try {
                    if (!decompiler.openProgram(currentProgram)) {
                        throw new IllegalStateException(decompiler.getLastMessage());
                    }
                    DecompileResults result = decompiler.decompileFunction(function, 30, monitor);
                    out.println("\nDecompilation completed: " + result.decompileCompleted());
                    out.println("Decompiler message: " + result.getErrorMessage());
                    if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
                        String code = result.getDecompiledFunction().getC();
                        out.println(code.substring(0, Math.min(code.length(), 8000)));
                    }
                }
                finally {
                    decompiler.dispose();
                }
            }
            if (out.checkError()) {
                throw new IllegalStateException("Failed writing startup inspection");
            }
        }
        println("Wrote startup inspection to " + args[0]);
    }
}
