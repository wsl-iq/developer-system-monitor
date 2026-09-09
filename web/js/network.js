/**
 * Network Monitor Component
 */

class NetworkViewer {
    static initialized = false;
    static refreshInterval = null;
    static chart = null;
    static maxDataPoints = 30;

    static initialize() {
        if (this.initialized) return;
        this.setupChart();
        this.initialized = true;
    }

    static setupChart() {
        const canvas = document.getElementById('network-chart-detail');
        if (!canvas || typeof Chart === 'undefined') {
            console.log('Network chart canvas not found');
            return;
        }

        const isDark = document.documentElement.getAttribute('data-theme') === 'dark';
        const gridColor = isDark ? 'rgba(255,255,255,0.05)' : 'rgba(0,0,0,0.05)';

        this.chart = new Chart(canvas, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Download (MB/s)',
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
                        label: 'Upload (MB/s)',
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
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index'
                },
                scales: {
                    x: {
                        grid: { color: gridColor, drawBorder: false },
                        ticks: { maxTicksLimit: 6, font: { size: 10 } }
                    },
                    y: {
                        beginAtZero: true,
                        grid: { color: gridColor, drawBorder: false },
                        ticks: { font: { size: 10 } }
                    }
                },
                plugins: {
                    legend: {
                        position: 'top',
                        labels: { boxWidth: 12, padding: 10, font: { size: 10 } }
                    }
                },
                animation: { duration: 300 }
            }
        });

        console.log('Network chart initialized');
    }

    static async load() {
        this.initialize();
        console.log('Loading network data...');

        try {
            const [adapters, stats] = await Promise.all([
                api.getNetworkAdapters(),
                api.getNetworkStats()
            ]);

            console.log(`Got ${Array.isArray(adapters) ? adapters.length : 0} adapters`);

            this.renderStats(stats);
            this.renderAdapters(adapters);

            this.startAutoRefresh();
        } catch (error) {
            console.error('Failed to load network info:', error);
            this.showEmptyState();
        }
    }

    static startAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
        }

        this.refreshInterval = setInterval(async () => {
            try {
                const adapters = await api.getNetworkAdapters();
                this.renderAdapters(adapters);
                this.updateChart(adapters);
            } catch (error) {
                // Silent fail
            }
        }, 5000);
    }

    static renderStats(stats) {
        const container = document.getElementById('network-stats-grid');
        if (!container) return;

        if (!stats) {
            container.innerHTML = '<div class="info-card"><p>No stats available</p></div>';
            return;
        }

        const statCards = [
            { icon: 'fa-network-wired', label: 'Total Adapters', value: stats.total_adapters || 0 },
            { icon: 'fa-check-circle', label: 'Active Adapters', value: stats.active_adapters || 0 },
            { icon: 'fa-arrow-down', label: 'Total Received', value: app.formatBytes(stats.total_bytes_received || 0) },
            { icon: 'fa-arrow-up', label: 'Total Sent', value: app.formatBytes(stats.total_bytes_sent || 0) },
        ];

        container.innerHTML = statCards.map(card => `
            <div class="info-card" style="padding: 15px; text-align: center;">
                <div style="font-size: 1.5rem; margin-bottom: 8px; color: var(--accent);">
                    <i class="fas ${card.icon}"></i>
                </div>
                <div style="font-size: 1.2rem; font-weight: 700; margin-bottom: 4px;">
                    ${card.value}
                </div>
                <div style="font-size: 0.8rem; color: var(--text-secondary);">
                    ${card.label}
                </div>
            </div>
        `).join('');
    }

    static renderAdapters(adapters) {
        const container = document.getElementById('network-adapters-list');
        if (!container) return;

        if (!Array.isArray(adapters) || adapters.length === 0) {
            this.showEmptyState();
            return;
        }

        container.innerHTML = adapters.map(adapter => {
            const statusClass = adapter.status === 'Up' ? 'status-running' : 'status-stopped';
            const statusIcon = adapter.status === 'Up' ? 'fa-check-circle' : 'fa-times-circle';
            const statusColor = adapter.status === 'Up' ? 'var(--success)' : 'var(--danger)';

            return `
                <div class="info-card">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px;">
                        <h3 style="margin: 0;">
                            <i class="fas fa-network-wired" style="color: var(--accent);"></i> 
                            ${this.escapeHtml(adapter.description || adapter.name || 'Adapter')}
                        </h3>
                        <span class="status-badge ${statusClass}" style="display: inline-flex; align-items: center; gap: 5px;">
                            <i class="fas ${statusIcon}" style="color: ${statusColor};"></i>
                            ${adapter.status || 'Unknown'}
                        </span>
                    </div>
                    
                    <table class="info-table" style="width: 100%;">
                        <tr>
                            <td style="color: var(--text-secondary); width: 40%;">MAC Address</td>
                            <td style="font-family: var(--font-mono);">${adapter.mac_address || 'Unknown'}</td>
                        </tr>
                        <tr>
                            <td style="color: var(--text-secondary);">Type</td>
                            <td>${adapter.adapter_type || 'Unknown'}</td>
                        </tr>
                        ${adapter.ip_addresses && adapter.ip_addresses.length > 0 ? `
                            <tr>
                                <td style="color: var(--text-secondary);">IP Addresses</td>
                                <td>${adapter.ip_addresses.join(', ')}</td>
                            </tr>
                        ` : ''}
                    </table>
                    
                    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(130px, 1fr)); gap: 10px; margin-top: 12px;">
                        <div style="background: var(--bg-tertiary); padding: 8px; border-radius: 6px; text-align: center;">
                            <div style="font-size: 0.7rem; color: var(--text-muted); margin-bottom: 3px;">
                                <i class="fas fa-arrow-down"></i> Download
                            </div>
                            <div style="font-weight: 600; font-size: 0.85rem;">
                                ${app.formatBytes(adapter.bytes_received_per_sec || 0)}/s
                            </div>
                        </div>
                        <div style="background: var(--bg-tertiary); padding: 8px; border-radius: 6px; text-align: center;">
                            <div style="font-size: 0.7rem; color: var(--text-muted); margin-bottom: 3px;">
                                <i class="fas fa-arrow-up"></i> Upload
                            </div>
                            <div style="font-weight: 600; font-size: 0.85rem;">
                                ${app.formatBytes(adapter.bytes_sent_per_sec || 0)}/s
                            </div>
                        </div>
                        <div style="background: var(--bg-tertiary); padding: 8px; border-radius: 6px; text-align: center;">
                            <div style="font-size: 0.7rem; color: var(--text-muted); margin-bottom: 3px;">
                                <i class="fas fa-database"></i> Total Received
                            </div>
                            <div style="font-weight: 600; font-size: 0.85rem;">
                                ${app.formatBytes(adapter.bytes_received || 0)}
                            </div>
                        </div>
                        <div style="background: var(--bg-tertiary); padding: 8px; border-radius: 6px; text-align: center;">
                            <div style="font-size: 0.7rem; color: var(--text-muted); margin-bottom: 3px;">
                                <i class="fas fa-database"></i> Total Sent
                            </div>
                            <div style="font-weight: 600; font-size: 0.85rem;">
                                ${app.formatBytes(adapter.bytes_sent || 0)}
                            </div>
                        </div>
                    </div>
                </div>
            `;
        }).join('');
    }

    static updateChart(adapters) {
        if (!this.chart || !Array.isArray(adapters)) return;

        let totalDownload = 0;
        let totalUpload = 0;

        adapters.forEach(adapter => {
            totalDownload += adapter.bytes_received_per_sec || 0;
            totalUpload += adapter.bytes_sent_per_sec || 0;
        });

        const now = new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });

        this.chart.data.labels.push(now);
        this.chart.data.datasets[0].data.push(Number((totalDownload / (1024 * 1024)).toFixed(4)));
        this.chart.data.datasets[1].data.push(Number((totalUpload / (1024 * 1024)).toFixed(4)));

        if (this.chart.data.labels.length > this.maxDataPoints) {
            this.chart.data.labels.shift();
            this.chart.data.datasets[0].data.shift();
            this.chart.data.datasets[1].data.shift();
        }

        this.chart.update('none');
    }

    static showEmptyState() {
        const container = document.getElementById('network-adapters-list');
        if (container) {
            container.innerHTML = `
                <div class="info-card" style="text-align: center; padding: 40px;">
                    <i class="fas fa-network-wired" style="font-size: 3rem; opacity: 0.3; margin-bottom: 15px;"></i>
                    <p style="color: var(--text-muted);">No network adapters found</p>
                </div>
            `;
        }
    }

    static formatSpeed(bps) {
        if (!bps) return '0 bps';
        const units = ['bps', 'Kbps', 'Mbps', 'Gbps'];
        const index = Math.floor(Math.log(bps) / Math.log(1000));
        if (index >= units.length) return bps + ' bps';
        return `${(bps / Math.pow(1000, index)).toFixed(2)} ${units[index]}`;
    }

    static escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str || '';
        return div.innerHTML;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { NetworkViewer };
}