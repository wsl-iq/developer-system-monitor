/**
 * Hardware Information Viewer Component
 */

class HardwareViewer {
    static initialized = false;

    static initialize() {
        if (this.initialized) return;
        this.initialized = true;
    }

    static async load() {
        this.initialize();

        try {
            const hardware = await api.getHardwareInfo();
            this.renderHardware(hardware);
        } catch (error) {
            console.error('Failed to load hardware info:', error);
        }
    }

    static renderHardware(hardware) {
        const container = document.getElementById('hardware-details');
        if (!container) return;

        let html = '';

        // CPU Section
        if (hardware.cpu) {
            html += this.renderCPU(hardware.cpu);
        }

        // GPU Section
        if (hardware.gpus && hardware.gpus.length > 0) {
            html += this.renderGPUs(hardware.gpus);
        }

        // Motherboard Section
        if (hardware.motherboard) {
            html += this.renderMotherboard(hardware.motherboard);
        }

        // BIOS Section
        if (hardware.bios) {
            html += this.renderBIOS(hardware.bios);
        }

        // Storage Devices
        if (hardware.storage_devices && hardware.storage_devices.length > 0) {
            html += this.renderStorage(hardware.storage_devices);
        }

        container.innerHTML = html || '<div class="info-card"><p>No hardware information available</p></div>';
    }

    static renderCPU(cpu) {
        return `
            <div class="info-card">
                <h3><i class="fas fa-brain"></i> CPU</h3>
                <table class="info-table">
                    <tr><td>Manufacturer</td><td>${cpu.manufacturer || 'Unknown'}</td></tr>
                    <tr><td>Model</td><td>${cpu.model || 'Unknown'}</td></tr>
                    <tr><td>Architecture</td><td>${cpu.architecture || 'Unknown'}</td></tr>
                    <tr><td>Cores</td><td>${cpu.cores || 0}</td></tr>
                    <tr><td>Threads</td><td>${cpu.threads || 0}</td></tr>
                    <tr><td>Frequency</td><td>${cpu.frequency_mhz || 0} MHz</td></tr>
                </table>
            </div>
        `;
    }

    static renderGPUs(gpus) {
        return gpus.map(gpu => `
            <div class="info-card">
                <h3><i class="fas fa-display"></i> GPU</h3>
                <table class="info-table">
                    <tr><td>Name</td><td>${gpu.name || 'Unknown'}</td></tr>
                    <tr><td>Vendor</td><td>${gpu.vendor || 'Unknown'}</td></tr>
                    <tr><td>VRAM</td><td>${app.formatBytes(gpu.vram_bytes || 0)}</td></tr>
                    <tr><td>Driver</td><td>${gpu.driver_version || 'Unknown'}</td></tr>
                </table>
            </div>
        `).join('');
    }

    static renderMotherboard(motherboard) {
        return `
            <div class="info-card">
                <h3><i class="fas fa-server"></i> Motherboard</h3>
                <table class="info-table">
                    <tr><td>Manufacturer</td><td>${motherboard.manufacturer || 'Unknown'}</td></tr>
                    <tr><td>Model</td><td>${motherboard.model || 'Unknown'}</td></tr>
                    <tr><td>Version</td><td>${motherboard.version || 'Unknown'}</td></tr>
                </table>
            </div>
        `;
    }

    static renderBIOS(bios) {
        return `
            <div class="info-card">
                <h3><i class="fas fa-microchip"></i> BIOS</h3>
                <table class="info-table">
                    <tr><td>Manufacturer</td><td>${bios.manufacturer || 'Unknown'}</td></tr>
                    <tr><td>Version</td><td>${bios.version || 'Unknown'}</td></tr>
                    <tr><td>Release Date</td><td>${bios.release_date || 'Unknown'}</td></tr>
                </table>
            </div>
        `;
    }

    static renderStorage(devices) {
        return `
            <div class="info-card" style="grid-column: 1/-1;">
                <h3><i class="fas fa-hard-drive"></i> Storage Devices</h3>
                <div class="table-container">
                    <table class="data-table">
                        <thead>
                            <tr>
                                <th>Name</th>
                                <th>Model</th>
                                <th>Interface</th>
                                <th>Type</th>
                                <th>Capacity</th>
                            </tr>
                        </thead>
                        <tbody>
                            ${devices.map(device => `
                                <tr>
                                    <td>${device.name || 'Unknown'}</td>
                                    <td>${device.model || 'Unknown'}</td>
                                    <td>${device.interface_type || 'Unknown'}</td>
                                    <td>${device.media_type || 'Unknown'}</td>
                                    <td>${app.formatBytes(device.capacity_bytes || 0)}</td>
                                </tr>
                            `).join('')}
                        </tbody>
                    </table>
                </div>
            </div>
        `;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { HardwareViewer };
}