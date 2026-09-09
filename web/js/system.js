/**
 * System Information Viewer
 * CPU, Memory, GPU, and Disk details
 */

class SystemViewer {
    static initialized = false;
    static initialize() {
        if (this.initialized) return;
        this.initialized = true;
    }

    static async loadCPU() {
        this.initialize();
        try {
            const cpu = await api.getCPUInfo();
            this.renderCPU(cpu);
        } catch (error) {
            console.error('Failed to load CPU info:', error);
        }
    }

    static renderCPU(cpu) {
        const container = document.getElementById('cpu-details');
        if (!container) return;

        const usage = cpu.total_usage_percent || 0;
        const usageClass = this.getUsageClass(usage);
        container.innerHTML = `
            <div class="info-card">
                <h3><i class="fas fa-brain"></i> Processor Information</h3>
                <table class="info-table">
                    <tr><td>Manufacturer</td><td>${cpu.manufacturer || 'Unknown'}</td></tr>
                    <tr><td>Model</td><td>${cpu.model || 'Unknown'}</td></tr>
                    <tr><td>Architecture</td><td>${cpu.architecture || 'Unknown'}</td></tr>
                    <tr><td>Physical Cores</td><td>${cpu.physical_cores || 0}</td></tr>
                    <tr><td>Logical Processors</td><td>${cpu.logical_processors || 0}</td></tr>
                    <tr><td>Current Frequency</td><td>${cpu.current_frequency_mhz || 0} MHz</td></tr>
                    <tr><td>Max Frequency</td><td>${cpu.max_frequency_mhz || 0} MHz</td></tr>
                    ${cpu.temperature_available ? `
                        <tr><td>Temperature</td><td>${cpu.temperature_celsius}°C</td></tr>
                    ` : ''}
                </table>
            </div>
            
            <div class="info-card">
                <h3><i class="fas fa-chart-bar"></i> Current Usage: ${usage.toFixed(1)}%</h3>
                <div class="progress-container">
                    <div class="progress-bar ${usageClass}" style="width: ${usage}%"></div>
                </div>
                
                ${cpu.core_usages && cpu.core_usages.length > 0 ? `
                    <h3 style="margin-top: 20px;"><i class="fas fa-microchip"></i> Per-Core Usage</h3>
                    <div class="core-grid">
                        ${cpu.core_usages.map((coreUsage, index) => `
                            <div class="core-item">
                                <div class="core-label">Core ${index}</div>
                                <div class="core-usage" style="color: ${this.getColor(coreUsage)}">
                                    ${coreUsage.toFixed(1)}%
                                </div>
                                <div class="progress-container" style="height: 4px; margin-top: 5px;">
                                    <div class="progress-bar ${this.getUsageClass(coreUsage)}" 
                                         style="width: ${coreUsage}%"></div>
                                </div>
                            </div>
                        `).join('')}
                    </div>
                ` : ''}
            </div>
        `;
    }

    static async loadMemory() {
        this.initialize();
        try {
            const memory = await api.getMemoryInfo();
            this.renderMemory(memory);
        } catch (error) {
            console.error('Failed to load memory info:', error);
        }
    }

    static renderMemory(memory) {
        const container = document.getElementById('memory-details');
        if (!container) return;

        const usage = memory.usage_percent || 0;
        const usageClass = this.getUsageClass(usage);

        container.innerHTML = `
            <div class="info-card">
                <h3><i class="fas fa-memory"></i> Memory Overview</h3>
                <table class="info-table">
                    <tr><td>Total Physical</td><td>${app.formatBytes(memory.total_physical_bytes)}</td></tr>
                    <tr><td>Used Physical</td><td>${app.formatBytes(memory.used_physical_bytes)}</td></tr>
                    <tr><td>Available</td><td>${app.formatBytes(memory.available_physical_bytes)}</td></tr>
                    <tr><td>Usage</td><td>${usage.toFixed(2)}%</td></tr>
                </table>
                
                <div class="progress-container" style="margin-top: 15px;">
                    <div class="progress-bar ${usageClass}" style="width: ${usage}%"></div>
                </div>
            </div>
            
            ${memory.modules && memory.modules.length > 0 ? `
                <div class="info-card">
                    <h3><i class="fas fa-list"></i> Memory Modules</h3>
                    <div class="table-container">
                        <table class="data-table">
                            <thead>
                                <tr>
                                    <th>Manufacturer</th>
                                    <th>Part Number</th>
                                    <th>Capacity</th>
                                    <th>Speed</th>
                                    <th>Type</th>
                                </tr>
                            </thead>
                            <tbody>
                                ${memory.modules.map(module => `
                                    <tr>
                                        <td>${module.manufacturer || 'Unknown'}</td>
                                        <td>${module.part_number || 'Unknown'}</td>
                                        <td>${app.formatBytes(module.capacity_bytes)}</td>
                                        <td>${module.speed_mhz || 0} MHz</td>
                                        <td>${module.type || 'Unknown'}</td>
                                    </tr>
                                `).join('')}
                            </tbody>
                        </table>
                    </div>
                </div>
            ` : ''}
        `;
    }

    static async loadGPU() {
        this.initialize();
        try {
            const gpuInfo = await api.getGPUInfo();
            this.renderGPU(gpuInfo);
        } catch (error) {
            console.error('Failed to load GPU info:', error);
        }
    }

    static renderGPU(gpuInfo) {
        const container = document.getElementById('gpu-details');
        if (!container) return;

        const gpus = gpuInfo.gpus || [];

        container.innerHTML = gpus.map(gpu => `
            <div class="info-card">
                <h3><i class="fas fa-display"></i> ${gpu.name || 'GPU'}</h3>
                <table class="info-table">
                    <tr><td>Vendor</td><td>${gpu.vendor || 'Unknown'}</td></tr>
                    <tr><td>VRAM</td><td>${app.formatBytes(gpu.vram_bytes)}</td></tr>
                    <tr><td>Driver</td><td>${gpu.driver_version || 'Unknown'}</td></tr>
                    <tr><td>Vendor ID</td><td>0x${(gpu.vendor_id || 0).toString(16)}</td></tr>
                    <tr><td>Device ID</td><td>0x${(gpu.device_id || 0).toString(16)}</td></tr>
                </table>
            </div>
        `).join('') || '<div class="info-card"><p>No GPU information available</p></div>';
    }

    static async loadDisk() {
        this.initialize();
        try {
            const disks = await api.getDisks();
            this.renderDisk(disks);
        } catch (error) {
            console.error('Failed to load disk info:', error);
        }
    }

    static renderDisk(disks) {
        const container = document.getElementById('disk-details');
        if (!container) return;

        if (!Array.isArray(disks) || disks.length === 0) {
            container.innerHTML = '<div class="info-card"><p>No disk information available</p></div>';
            return;
        }

        container.innerHTML = `
            <div class="info-card" style="grid-column: 1/-1;">
                <h3><i class="fas fa-hard-drive"></i> Disk Drives</h3>
                <div class="table-container">
                    <table class="data-table">
                        <thead>
                            <tr>
                                <th>Drive</th>
                                <th>Name</th>
                                <th>Type</th>
                                <th>File System</th>
                                <th>Total</th>
                                <th>Free</th>
                                <th>Usage</th>
                            </tr>
                        </thead>
                        <tbody>
                            ${disks.map(disk => {
                                const usage = disk.usage_percent || 0;
                                const usageClass = this.getUsageClass(usage);
                                return `
                                    <tr>
                                        <td><strong>${disk.drive_letter || '?'}</strong></td>
                                        <td>${disk.volume_name || 'Unknown'}</td>
                                        <td>${disk.drive_type || 'Unknown'}</td>
                                        <td>${disk.file_system || 'Unknown'}</td>
                                        <td>${app.formatBytes(disk.total_bytes)}</td>
                                        <td>${app.formatBytes(disk.free_bytes)}</td>
                                        <td>
                                            <div class="progress-container" style="width: 100px; display: inline-block; margin: 0 5px;">
                                                <div class="progress-bar ${usageClass}" style="width: ${usage}%"></div>
                                            </div>
                                            ${usage.toFixed(1)}%
                                        </td>
                                    </tr>
                                `;
                            }).join('')}
                        </tbody>
                    </table>
                </div>
            </div>
        `;
    }

    static getUsageClass(usage) {
        if (usage < 50) return 'low';
        if (usage < 80) return 'medium';
        return 'high';
    }

    static getColor(usage) {
        if (usage < 50) return 'var(--success)';
        if (usage < 80) return 'var(--warning)';
        return 'var(--danger)';
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { SystemViewer };
}