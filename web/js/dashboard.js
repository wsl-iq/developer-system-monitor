/**
 * Dashboard Component
 * Real-time system metrics with charts
 */

class Dashboard {
    static charts = {};
    static metrics = {};
    static initialized = false;
    static maxDataPoints = 30;
    
    static initialize() {
        if (this.initialized) return;
        
        this.setupCharts();
        this.setupEventListeners();
        this.initialized = true;
    }
    
    static setupCharts() {
        // CPU Chart
        const cpuCanvas = document.getElementById('cpu-chart');
        if (cpuCanvas && typeof Chart !== 'undefined') {
            this.charts.cpu = new Chart(cpuCanvas, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'CPU',
                        data: [],
                        borderColor: '#6366f1',
                        backgroundColor: 'rgba(99, 102, 241, 0.1)',
                        tension: 0.4,
                        fill: true,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 5
                    }]
                },
                options: this.getChartOptions('Percentage (%)')
            });
        }
        
        // Memory Chart
        const memoryCanvas = document.getElementById('memory-chart');
        if (memoryCanvas && typeof Chart !== 'undefined') {
            this.charts.memory = new Chart(memoryCanvas, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Memory',
                        data: [],
                        borderColor: '#10b981',
                        backgroundColor: 'rgba(16, 185, 129, 0.1)',
                        tension: 0.4,
                        fill: true,
                        borderWidth: 2,
                        pointRadius: 0,
                        pointHoverRadius: 5
                    }]
                },
                options: this.getChartOptions('Percentage (%)')
            });
        }
        
        // Network Chart
        const networkCanvas = document.getElementById('network-chart');
        if (networkCanvas && typeof Chart !== 'undefined') {
            this.charts.network = new Chart(networkCanvas, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [
                        {
                            label: 'Download',
                            data: [],
                            borderColor: '#3b82f6',
                            backgroundColor: 'rgba(59, 130, 246, 0.1)',
                            tension: 0.4,
                            fill: true,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 5
                        },
                        {
                            label: 'Upload',
                            data: [],
                            borderColor: '#10b981',
                            backgroundColor: 'rgba(16, 185, 129, 0.1)',
                            tension: 0.4,
                            fill: true,
                            borderWidth: 2,
                            pointRadius: 0,
                            pointHoverRadius: 5
                        }
                    ]
                },
                options: this.getChartOptions('Speed (MB/s)')
            });
        }
    }
    
    static getChartOptions(yLabel) {
        const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
        
        return {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                intersect: false,
                mode: 'index'
            },
            scales: {
                x: {
                    display: true,
                    grid: {
                        color: isDark ? 'rgba(255,255,255,0.05)' : 'rgba(0,0,0,0.05)',
                        drawBorder: false
                    },
                    ticks: {
                        maxTicksLimit: 6,
                        font: { size: 10 }
                    }
                },
                y: {
                    display: true,
                    beginAtZero: true,
                    grid: {
                        color: isDark ? 'rgba(255,255,255,0.05)' : 'rgba(0,0,0,0.05)',
                        drawBorder: false
                    },
                    ticks: {
                        font: { size: 10 }
                    },
                    title: {
                        display: true,
                        text: yLabel,
                        font: { size: 10 }
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top',
                    labels: {
                        boxWidth: 12,
                        padding: 10,
                        font: { size: 10 }
                    }
                }
            },
            animation: {
                duration: 300
            }
        };
    }
    
    static setupEventListeners() {
        document.addEventListener('metrics-update', (event) => {
            this.updateMetrics(event.detail);
        });
    }
    
    static async load() {
        this.initialize();
        
        try {
            if (typeof api !== 'undefined') {
                // Load initial data
                const [cpuInfo, memoryInfo, disks, network, processes] = await Promise.all([
                    api.getCPUInfo(),
                    api.getMemoryInfo(),
                    api.getDisks(),
                    api.getNetworkAdapters(),
                    api.getProcesses()
                ]);
                
                const metrics = {
                    timestamp: new Date().toISOString(),
                    cpu: cpuInfo,
                    memory: memoryInfo,
                    disks: disks,
                    network: network
                };
                
                this.updateMetrics(metrics);
                
                // Update process count
                if (processes) {
                    const countElement = document.getElementById('process-count-value');
                    if (countElement) countElement.textContent = processes.length;
                }
            }
        } catch (error) {
            console.error('Failed to load dashboard data:', error);
        }
    }
    
    static updateMetrics(data) {
        this.metrics = data;
        
        if (data.cpu) this.updateCPU(data.cpu);
        if (data.memory) this.updateMemory(data.memory);
        if (data.network) this.updateNetwork(data.network);
        if (data.disks) this.updateDisk(data.disks);
        if (data.timestamp) this.updateTimestamp(data.timestamp);
    }
    
    static updateCPU(cpu) {
        const usage = cpu.total_usage_percent || cpu.usage_percent || 0;
        
        // Update value
        const valueElement = document.getElementById('cpu-usage-value');
        if (valueElement) valueElement.textContent = `${usage.toFixed(1)}%`;
        
        // Update progress bar
        const progressBar = document.getElementById('cpu-progress-bar');
        if (progressBar) {
            progressBar.style.width = `${usage}%`;
            progressBar.className = `progress-bar ${this.getUsageClass(usage)}`;
        }
        
        // Update frequency
        const freqElement = document.getElementById('cpu-frequency');
        if (freqElement && cpu.current_frequency_mhz) {
            freqElement.textContent = `${cpu.current_frequency_mhz} MHz`;
        }
        
        // Update temperature
        const tempElement = document.getElementById('cpu-temp');
        if (tempElement && cpu.temperature_available && cpu.temperature_celsius) {
            tempElement.textContent = `${cpu.temperature_celsius}°C`;
        }
        
        // Update chart
        if (this.charts.cpu) {
            this.updateChart(this.charts.cpu, usage);
            const legend = document.getElementById('cpu-chart-legend');
            if (legend) legend.textContent = `${usage.toFixed(1)}%`;
        }
    }
    
    static updateMemory(memory) {
        const usage = memory.usage_percent || 0;
        
        const valueElement = document.getElementById('memory-usage-value');
        if (valueElement) valueElement.textContent = `${usage.toFixed(1)}%`;
        
        const progressBar = document.getElementById('memory-progress-bar');
        if (progressBar) {
            progressBar.style.width = `${usage}%`;
            progressBar.className = `progress-bar ${this.getUsageClass(usage)}`;
        }
        
        const usedElement = document.getElementById('memory-used');
        if (usedElement && memory.used_bytes) {
            usedElement.textContent = this.formatGB(memory.used_bytes);
        }
        
        const totalElement = document.getElementById('memory-total');
        if (totalElement && memory.total_bytes) {
            totalElement.textContent = this.formatGB(memory.total_bytes);
        }
        
        if (this.charts.memory) {
            this.updateChart(this.charts.memory, usage);
            const legend = document.getElementById('memory-chart-legend');
            if (legend) legend.textContent = `${usage.toFixed(1)}%`;
        }
    }
    
    static updateNetwork(network) {
        let totalDownload = 0;
        let totalUpload = 0;
        
        if (Array.isArray(network)) {
            network.forEach(adapter => {
                totalDownload += adapter.bytes_received_per_sec || 0;
                totalUpload += adapter.bytes_sent_per_sec || 0;
            });
        }
        
        const downloadElement = document.getElementById('network-download');
        if (downloadElement) {
            downloadElement.textContent = `${this.formatSpeed(totalDownload)}`;
        }
        
        const uploadElement = document.getElementById('network-upload');
        if (uploadElement) {
            uploadElement.textContent = `${this.formatSpeed(totalUpload)}`;
        }
        
        // Update chart with MB/s values
        if (this.charts.network) {
            const downloadMB = totalDownload / (1024 * 1024);
            const uploadMB = totalUpload / (1024 * 1024);
            
            this.updateNetworkChart(downloadMB, uploadMB);
            
            const downloadStat = document.getElementById('chart-download-stat');
            const uploadStat = document.getElementById('chart-upload-stat');
            if (downloadStat) downloadStat.textContent = this.formatSpeed(totalDownload);
            if (uploadStat) uploadStat.textContent = this.formatSpeed(totalUpload);
        }
        
        // Update adapter count
        const adapterCount = document.getElementById('network-adapters-count');
        if (adapterCount && Array.isArray(network)) {
            adapterCount.textContent = `${network.length} adapters`;
        }
    }
    
    static updateDisk(disks) {
        const container = document.getElementById('disk-chart-container');
        if (!container || !Array.isArray(disks)) return;
        
        container.innerHTML = '';
        
        disks.forEach(disk => {
            const usage = disk.usage_percent || 0;
            const usageClass = this.getUsageClass(usage);
            const barColor = usage < 50 ? 'var(--success)' :
                             usage < 80 ? 'var(--warning)' :
                             'var(--danger)';
            
            const diskItem = document.createElement('div');
            diskItem.className = 'disk-item';
            diskItem.innerHTML = `
                <div class="disk-item-header">
                    <span class="disk-item-name">
                        <i class="fas fa-hard-drive"></i> ${disk.drive_letter || disk.name || 'Drive'}
                    </span>
                    <span class="disk-item-usage" style="color: ${barColor}; font-weight: 700;">${usage.toFixed(1)}%</span>
                </div>
                <div class="disk-item-bar" style="background-color: var(--bg-tertiary); border-radius: 4px; height: 20px; overflow: hidden;">
                    <div class="disk-item-bar-fill ${usageClass}" style="width: ${usage}%; height: 100%; background: ${barColor}; border-radius: 4px; transition: width 0.5s ease;"></div>
                </div>
                <div class="disk-item-details">
                    <span>${this.formatGB(disk.used_bytes || 0)} / ${this.formatGB(disk.total_bytes || 0)}</span>
                    <span>${this.formatGB(disk.free_bytes || 0)} free</span>
                </div>
            `;
            
            container.appendChild(diskItem);
        });
    }
    
    static updateTimestamp(timestamp) {
        const element = document.getElementById('last-update-time');
        if (element && timestamp) {
            const date = new Date(timestamp);
            element.textContent = date.toLocaleTimeString();
        }
    }
    
    static updateChart(chart, value) {
        const now = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
        
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(value);
        
        if (chart.data.labels.length > this.maxDataPoints) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
        }
        
        chart.update('none');
    }
    
    static updateNetworkChart(download, upload) {
        const chart = this.charts.network;
        if (!chart) return;
        
        const now = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
        
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(download);
        chart.data.datasets[1].data.push(upload);
        
        if (chart.data.labels.length > this.maxDataPoints) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
            chart.data.datasets[1].data.shift();
        }
        
        chart.update('none');
    }
    
    static getUsageClass(usage) {
        if (usage < 50) return 'low';
        if (usage < 80) return 'medium';
        return 'high';
    }
    
    static formatGB(bytes) {
        if (!bytes) return app.language === 'ar' ? '0 ج.ب' : '0 GB';
        if (app.language === 'ar') {
            return `${(bytes / (1024 ** 3)).toFixed(2)} ج.ب`;
        }
        return `${(bytes / (1024 ** 3)).toFixed(2)} GB`;
    }
    
    static formatSpeed(bytesPerSecond) {
        if (!bytesPerSecond) return app.language === 'ar' ? '0 ب/ث' : '0 B/s';
        
        const units = app.language === 'ar' ? 
            ['ب/ث', 'ك.ب/ث', 'م.ب/ث', 'ج.ب/ث'] : 
            ['B/s', 'KB/s', 'MB/s', 'GB/s'];
        
        const index = Math.floor(Math.log(bytesPerSecond) / Math.log(1024));
        
        if (index >= units.length) return bytesPerSecond + (app.language === 'ar' ? ' ب/ث' : ' B/s');
        if (index < 0) return app.language === 'ar' ? '0 ب/ث' : '0 B/s';
        
        return `${(bytesPerSecond / Math.pow(1024, index)).toFixed(2)} ${units[index]}`;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { Dashboard };
}
